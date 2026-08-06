/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <algorithm>	// min
#include <cmath>
#include "Transformer.h"
#include "MatrixOperations.h"
#include "WarpConfiguratorUtils.h"

namespace intel_vvp_warp
{

namespace
{
static inline bool TFloatNotEqual(float f1, float f2)
{
	return ((float)fabs(f1 - f2) >= 0.00001f);
}
static inline bool TFloatNotEqualZero(float f1)
{
	return TFloatNotEqual(f1, 0);
}
}

TransformerAffine::TransformerAffine(float ratio):
	ITransformer(),
	_ratio(ratio),
	_origin{0.5f, _ratio/2.0f}
{
	Reset();
}

TransformerAffine::~TransformerAffine()
{

}

// Points are normalised in the range [0..1]
// However for the rotation to work correctly
// y-coordinate is prescaled using output resolution aspect ratio
// after transformation y-coordinate is scaled back
void TransformerAffine::Transform(float& x, float& y, uint32_t /*idx*/)
{
	vector4f_t v_in = {x, y*_ratio, 0, 1};
	vector4f_t v_out;

#ifdef NEON
	// inline asm does the following:
	// load transformation matrix into d0-d7 registers
	// load input vector v_in into d8-d9 registers
	// multiply and accumulate product in q12 register
	// store product to v_out
	__asm__ volatile (
	    "vld1.32  {d0-d3}, [%[matrix]]!\n\t"
	    "vld1.32  {d4-d7}, [%[matrix]]!\n\t"
	    "vld1.32  {d8-d9}, [%[tmp_in]]!\n\t"
	    "vmul.f32 q12, q0, d8[0]\n\t"
	    "vmla.f32 q12, q1, d8[1]\n\t"
	    "vmla.f32 q12, q2, d9[0]\n\t"
	    "vmla.f32 q12, q3, d9[1]\n\t"

	    "vst1.32  {d24-d25}, [%[tmp_out]]!"
	    :
	    :[matrix] "r" (&_matrix[0]),[tmp_in] "rm" (&v_in[0]),[tmp_out] "rm" (&v_out[0])
	    :"d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","q12","memory"
	);

#else
	v_out[0] = (_matrix[0] * v_in[0] + _matrix[4] * v_in[1] + _matrix[8 ] * v_in[2] + _matrix[12] * v_in[3]);
	v_out[1] = (_matrix[1] * v_in[0] + _matrix[5] * v_in[1] + _matrix[9 ] * v_in[2] + _matrix[13] * v_in[3]);
	v_out[2] = (_matrix[2] * v_in[0] + _matrix[6] * v_in[1] + _matrix[10] * v_in[2] + _matrix[14] * v_in[3]);
	v_out[3] = (_matrix[3] * v_in[0] + _matrix[7] * v_in[1] + _matrix[11] * v_in[2] + _matrix[15] * v_in[3]);
#endif /*NEON*/

	x = v_out[0];
	y = v_out[1]/_ratio;
}


void TransformerAffine::Reset()
{
	_hoff = 0.0;
	_voff = 0.0;
	_zoom = 1.0;
	_hzoom = 1.0;
	_vzoom = 1.0;
	_theta = 0.0;

	_hsize = 1.0f;
	_vsize = _ratio;

	_hmirr = 1.0f;
	_vmirr = 1.0f;

	matrix_t identity_matrix =
	{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	MatrixOperations::CopyMatrix(_matrix, identity_matrix);

	MatrixOperations::CopyMatrix(_matrix_translation1, identity_matrix);
	_matrix_translation1[12] = _origin[0];
	_matrix_translation1[13] = _origin[1];

	MatrixOperations::CopyMatrix(_matrix_translation2, identity_matrix);
	_matrix_translation2[12] = -_origin[0];
	_matrix_translation2[13] = -_origin[1];

	MatrixOperations::CopyMatrix(_matrix_rotation, identity_matrix);
	MatrixOperations::CopyMatrix(_matrix_zoom, identity_matrix);
}

void TransformerAffine::UpdateRatio(float ratio)
{
	_ratio = ratio;
	_origin[1] = _ratio/2.0f;

	_matrix_translation1[13] = _origin[1]+_voff;
	_matrix_translation2[13] = -_origin[1];

	RecalculateMatrix();
}

void TransformerAffine::SetRotate(float a)
{
	_theta = a;

	float cos_theta = cosf(_theta);
	float sin_theta = sinf(_theta);

	_matrix_rotation[0] = cos_theta;
	_matrix_rotation[1] = -sin_theta;
	_matrix_rotation[4] = sin_theta;
	_matrix_rotation[5] = cos_theta;

	RecalculateMatrix();
}

void TransformerAffine::SetHOffset(float v)
{
	_hoff = v;
	_matrix_translation1[12] = _origin[0]+_hoff;
	RecalculateMatrix();
}

void TransformerAffine::SetVOffset(float v)
{
	_voff = v * _ratio;
	_matrix_translation1[13] = _origin[1]+_voff;
	RecalculateMatrix();
}

void TransformerAffine::SetZoom(float v)
{
	_zoom = v;

	UpdateZoomMatrix();
	RecalculateMatrix();
}

void TransformerAffine::SetHSize(float value)
{
	_hsize = value;
	_hzoom = _hsize;

	UpdateZoomMatrix();
	RecalculateMatrix();
}

void TransformerAffine::SetVSize(float value)
{
	_vsize = value * _ratio;
	_vzoom = _vsize/_ratio;

	UpdateZoomMatrix();
	RecalculateMatrix();
}

void TransformerAffine::SetHMirror(bool v)
{
	_hmirr = v ? -1.0f : 1.0f;

	UpdateZoomMatrix();
	RecalculateMatrix();
}

void TransformerAffine::SetVMirror(bool v)
{
	_vmirr = v ? -1.0f : 1.0f;

	UpdateZoomMatrix();
	RecalculateMatrix();
}

void TransformerAffine::UpdateZoomMatrix()
{
	// Note zoom coefficient locations are the same
	// for both row and column major orders
	_matrix_zoom[0] = (_hmirr * _hzoom * _zoom);
	_matrix_zoom[5] = (_vmirr * _vzoom *_zoom);
}

void TransformerAffine::RecalculateMatrix()
{
	// Translate rotate and restore
	// using one matrix which is a product
	// of three consequtive transforms
	matrix_t tmp_product;

	MatrixOperations::MatrixProduct4x4(_matrix_translation1, _matrix_rotation, _matrix);
	MatrixOperations::MatrixProduct4x4(_matrix, _matrix_zoom, tmp_product);
	MatrixOperations::MatrixProduct4x4(tmp_product, _matrix_translation2, _matrix);
}

//////////////////////
arc_conf_t::arc_conf_t()
	: _edge{0.0f, 0.0f, 0.0f, 0.0f}
{
}

bool arc_conf_t::IsConfigured() const
{
	return (TFloatNotEqualZero(_edge[0]) || TFloatNotEqualZero(_edge[1]) || TFloatNotEqualZero(_edge[2]) || TFloatNotEqualZero(_edge[3]));
}

void arc_conf_t::Reset()
{
	_edge[0] = 0.0f;
	_edge[1] = 0.0f;
	_edge[2] = 0.0f;
	_edge[3] = 0.0f;
}


//////////////////////

TransformerArc::TransformerArc(const arc_conf_t& arc_conf, bool reverse, bool within_bounding_box):
	ITransformer(),
	_reverse(reverse),
	_mx(within_bounding_box ? 1.0f : 2.0f),
	_dt(arc_conf._edge[0] * _mx),
	_db(arc_conf._edge[1] * _mx),
	_dl(arc_conf._edge[2] * _mx),
	_dr(arc_conf._edge[3] * _mx),

	_t_offs((_dt < 0.0f) ? -_dt : 0.0f),
	_b_offs((_db < 0.0f) ? -_db : 0.0f),
	_l_offs((_dl < 0.0f) ? -_dl : 0.0f),
	_r_offs((_dr < 0.0f) ? -_dr : 0.0f),

	_pt0{0.0f, 0.0f + (within_bounding_box ? (_dt + _t_offs) : 0)},
	_pt1{0.5f, 0.0f - _dt + (within_bounding_box ? _t_offs : 0)},
	_pt2{1.0f, 0.0f + (within_bounding_box ? (_dt + _t_offs) : 0)},

	_pb0{0.0f, 1.0f - (within_bounding_box ? (_db + _b_offs) : 0)},
	_pb1{0.5f, 1.0f + _db - (within_bounding_box ? _b_offs : 0)},
	_pb2{1.0f, 1.0f - (within_bounding_box ? (_db + _b_offs) : 0)},

	_pl0{0.0f + (within_bounding_box ? (_dl + _l_offs) : 0), 0.0f},
	_pl1{0.0f - _dl + (within_bounding_box ? _l_offs : 0), 0.5f},
	_pl2{0.0f + (within_bounding_box ? (_dl + _l_offs) : 0), 01.0f},

	_pr0{1.0f - (within_bounding_box ? (_dr + _r_offs) : 0), 0.0f},
	_pr1{1.0f + _dr - (within_bounding_box ? _r_offs : 0), 0.5f},
	_pr2{1.0f - (within_bounding_box ? (_dr + _r_offs) : 0), 1.0f},

	_pt{0.0},
	_pb{0.0},
	_pl{0.0},
	_pr{0.0}
{
}

TransformerArc::~TransformerArc()
{

}

void TransformerArc::Transform(float& x, float& y, uint32_t /*idx*/)
{
	// Vertical component
	// Top
	GetPointOnCurve(_pt0, _pt1, _pt2, x, _pt);

	// Bottom
	GetPointOnCurve(_pb0, _pb1, _pb2, x, _pb);

	point_t pv;

	if(_reverse)
	{
		float k = 1.0f/(_pb[1] - _pt[1]);
		float offset = 1.0f - k * _pb[1];
		float t = k * y + offset;
		point_t tmpp1{0,0};
		point_t tmpp2{0,1};
		_li(tmpp1, tmpp2, t, pv);
	}
	else
		_li(_pt, _pb, y, pv);

	// Horizontal component
	// Left
	GetPointOnCurve(_pl0, _pl1, _pl2, y, _pl);

	// Right
	GetPointOnCurve(_pr0, _pr1, _pr2, y, _pr);

	point_t ph;

	if(_reverse)
	{
		float k = 1.0f/(_pr[0] - _pl[0]);
		float offset = 1.0f - k * _pr[0];
		float t = k * x + offset;

		point_t tmpp1{0,0};
		point_t tmpp2{1,0};
		_li(tmpp1, tmpp2, t, ph);
	}
	else
		_li(_pl, _pr, x, ph);

	x = ph[0];
	y = pv[1];
}


TransformerRadial::TransformerRadial(float ratio, const point_t& focus, float k1, float k2) :
	ITransformer(),
	_ratio(ratio),
	_focus{ focus[0], focus[1] * _ratio },
	_k1(k1), _k2(k2)
{
}

TransformerRadial::~TransformerRadial()
{
}

// Points are normalised in the range [0..1]
// However for the fisheye to work correctly
// y-coordinate is prescaled using output resolution aspect ratio
// after transformation y-coordinate is scaled back
void TransformerRadial::Transform(float& x, float& y, uint32_t /*idx*/)
{
	y *= _ratio;

	const float dx = x - _focus[0];
	const float dy = y - _focus[1];
	const float r2 = dx * dx + dy * dy;
	const float m = r2 * _k1 + (r2 * r2) * _k2;
	
	x = x + dx * m;
	y = y + dy * m;
	y /= _ratio;
}


TransformerRadialInverse::TransformerRadialInverse(float ratio, const point_t& focus, float k1, float k2) :
	ITransformer(),
	_ratio(ratio),
	_focus{ focus[0], focus[1] * _ratio },
	_k1(k1), _k2(k2)
{
}

TransformerRadialInverse::~TransformerRadialInverse()
{

}

// Points are normalised in the range [0..1]
// However for the fisheye to work correctly
// y-coordinate is prescaled using output resolution aspect ratio
// after transformation y-coordinate is scaled back
void TransformerRadialInverse::Transform(float& x, float& y, uint32_t /*idx*/)
{
	y *= _ratio;
	const float orig_y = y;
	const float orig_x = x;

	// To find the inverse of the distortion model we use approximation
	// 12 iterations give accurate enough result
	// Higher number increases accuracy but also the processing time
	for (int i = 0; i < 15; i++)
	{
		const float dx = x - _focus[0];
		const float dy = y - _focus[1];
        const float r2 = dx * dx + dy * dy;
		const float m = r2 * _k1 + (r2 * r2) * _k2;

		x = ((_focus[0] * m) + orig_x) / (1 + m);
		y = ((_focus[1] * m) + orig_y) / (1 + m);
	}

	y /= _ratio;  
}


TransformerKeystone::TransformerKeystone(float ratio):
	ITransformer(),
	_FOV(degrees_to_radians(30.0f)),
	_ratio(ratio),
	_img_width(1.0f),
	_img_height(_img_width * _ratio),
	_vkey(0.0f),
	_hkey(0.0f),
	_voff(0.0f),
	_DEFAULT_POINTS{{0,0},{_img_width,0},{0,_img_height},{_img_width,_img_height}}
{
	Reset();
}

TransformerKeystone::~TransformerKeystone()
{

}

void TransformerKeystone::Reset()
{
	CopyPoints(_ip, _DEFAULT_POINTS, ENumCorners);

	_vkey = 0.0f;
	_hkey = 0.0f;
}


void TransformerKeystone::UpdateRatio(float ratio)
{
	_ratio = ratio;
	_img_height = _img_width * _ratio;
	_DEFAULT_POINTS[2][1] = _img_height;
	_DEFAULT_POINTS[3][1] = _img_height;
	UpdateIntersectionPoints();
}


// Points are normalised in the range [0..1]
// That allows to map into the input coordinate system
// achieving anamorphic fit
void TransformerKeystone::Transform(float& x, float& y, uint32_t idx/* = 0*/)
{
	x = _ip[idx][0];
	y = _ip[idx][1]/_ratio;
}


void TransformerKeystone::SetVKeystone(float a)
{
	_vkey = a;
	UpdateIntersectionPoints();
}


void TransformerKeystone::SetHKeystone(float a)
{
	_hkey = a;
	UpdateIntersectionPoints();
}


void TransformerKeystone::SetFOV(float a)
{
	_FOV = a;
	UpdateIntersectionPoints();
}


void TransformerKeystone::SetVAxisOffset(float v)
{
	_voff = v;
	UpdateIntersectionPoints();
}


void TransformerKeystone::UpdateIntersectionPoints()
{
	float pitch = _hkey;
	float roll = _vkey;

	float half_w = _img_width / 2.0f;
	float half_h = _img_height / 2.0f;

	float voff = _voff * _img_height;
	float top = -half_h - voff;
	float bot = half_h - voff;
	point_t src_points[ENumPoints] = {{-half_w, top}, {half_w, top},{-half_w, bot},{half_w, bot},{0.0f, -voff}};
	point_t dst_points[ENumPoints] = {{-half_w, top}, {half_w, top},{-half_w, bot},{half_w, bot},{0.0f, -voff}};

	// Focal length (z coordinate)
	float r = sqrtf(half_w * half_w + half_h * half_h);
	float source_z = r / tanf(_FOV/2.0f);

	matrix_t rmatrix;
	MatrixOperations::RotationMatrix(0.0f, pitch, roll, rmatrix);

	vector4f_t source_vectors[ENumPoints];

	for (int ct = 0; ct < ENumPoints; ct++)
	{
		source_vectors[ct][0] = src_points[ct][0];
		source_vectors[ct][1] = src_points[ct][1];
		source_vectors[ct][2] = source_z;
		source_vectors[ct][3] = 1.0f;
	}

	// Get projection plane from first three points
	plane_t source_plane = MatrixOperations::PlaneFromThreePoints(source_vectors[0], source_vectors[1], source_vectors[2]);
	vector4f_t origin_vector = { 0.0f, 0.0f, 0.0f, 1.0f };

	for (int ct = 0; ct < ENumPoints; ct++)
	{
		// Rotate the corner and find where it intersects the source plane
		vector4f_t rotated_vector;
		MatrixOperations::VectorMatrixColumnProduct(source_vectors[ct], rmatrix, rotated_vector);
		vector4f_t intersection_vector;
		MatrixOperations::PlaneLineIntersection(source_plane, origin_vector, rotated_vector, intersection_vector);

		dst_points[ct][0] = intersection_vector[0];
		dst_points[ct][1] = intersection_vector[1];
	}

	// Recenter projection for better transform accuracy
	for(int ct = 0; ct < ENumPoints; ++ct)
	{
		dst_points[ct][0] -= dst_points[ENumPoints-1][0];
		dst_points[ct][1] -= dst_points[ENumPoints-1][1];
		src_points[ct][0] -= src_points[ENumPoints-1][0];
		src_points[ct][1] -= src_points[ENumPoints-1][1];
	}

	// Copy only corner points, not the center point
	CopyPoints(_ip, src_points, ENumCorners);

	matrix_t matrix;
	MatrixOperations::DeriveMatrix(dst_points, src_points, matrix);
	std::vector<float> m(&matrix[0], &matrix[16]);

	TransformerPerspective t(m);

	t.Transform(_ip[0][0], _ip[0][1]);
	t.Transform(_ip[1][0], _ip[1][1]);
	t.Transform(_ip[2][0], _ip[2][1]);
	t.Transform(_ip[3][0], _ip[3][1]);

	// Fit warp in the screen
	float minx = _ip[0][0];
	float miny = _ip[0][1];
	float maxx = _ip[0][0];
	float maxy = _ip[0][1];

	for(int i=0; i < ENumCorners; ++i)
	{
		if(_ip[i][0] < minx) minx = _ip[i][0];
		if(_ip[i][1] < miny) miny = _ip[i][1];
		if(_ip[i][0] > maxx) maxx = _ip[i][0];
		if(_ip[i][1] > maxy) maxy = _ip[i][1];
	}

	float sx = fabsf(maxx - minx)/_img_width;
	float sy = fabsf(maxy - miny)/_img_height;
	float s = 1/std::max(sx,sy);

	// Scale down
	for(int i=0; i < ENumCorners; ++i)
	{
		_ip[i][0] *= s;
		_ip[i][1] *= s;
	}

	minx *= s;
	miny *= s;
	maxx *= s;
	maxy *= s;

	// Align distortion
	float h_off = (_hkey > 0.0f) ? (minx + half_w) : (maxx - half_w);
	float v_off = (_vkey > 0.0f) ? (maxy - half_h) : (miny + half_h);

	for(int i=0; i < ENumCorners; ++i)
	{
		_ip[i][0] += (half_w - h_off);
		_ip[i][1] += (half_h - v_off);
	}
}


TransformerPerspective::TransformerPerspective(const std::vector<float>& matrix):
	ITransformer(),
	_matrix(matrix)
{
}

TransformerPerspective::TransformerPerspective(const matrix_t& matrix):
	ITransformer(),
	_matrix(&matrix[0], &matrix[16])
{
}


TransformerPerspective::TransformerPerspective():
	ITransformer()
{
	static const float m[17] = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0};
	_matrix.assign(&m[0],&m[16]);
}

TransformerPerspective::~TransformerPerspective()
{
}

void TransformerPerspective::SetMatrix(const std::vector<float>& matrix)
{
	_matrix.assign(matrix.begin(), matrix.end());
}

void TransformerPerspective::Transform(float& x, float& y, uint32_t /*idx*/)
{
	vector4f_t input_point;
	vector4f_t output_point;

	input_point[0] = x;
	input_point[1] = y;
	input_point[2] = 0;
	input_point[3] = 1;

	Multiply(input_point, output_point);

	x = (output_point[0] / output_point[3]);
	y = (output_point[1] / output_point[3]);

	return;
}

void TransformerPerspective::Multiply(const vector4f_t& v_in, vector4f_t& v_out)
{
#ifdef NEON
	// inline asm does the following:
	// load transformation matrix into d0-d7 registers
	// load input vector v_in into d8-d9 registers
	// multiply and accumulate product in q12 register
	// store product to v_out
	__asm__ volatile (
	    "vld1.32  {d0-d3}, [%[matrix]]!\n\t"
	    "vld1.32  {d4-d7}, [%[matrix]]!\n\t"
	    "vld1.32  {d8-d9}, [%[tmp_in]]!\n\t"
	    "vmul.f32 q12, q0, d8[0]\n\t"
	    "vmla.f32 q12, q1, d8[1]\n\t"
	    "vmla.f32 q12, q2, d9[0]\n\t"
	    "vmla.f32 q12, q3, d9[1]\n\t"

	    "vst1.32  {d24-d25}, [%[tmp_out]]!"
	    :
	    :[matrix] "r" (&_matrix[0]),[tmp_in] "rm" (&v_in[0]),[tmp_out] "rm" (&v_out[0])
	    :"d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","q12","memory"
	);

#else
	v_out[0] = (_matrix[0] * v_in[0] + _matrix[4] * v_in[1] + _matrix[8 ] * v_in[2] + _matrix[12] * v_in[3]);
	v_out[1] = (_matrix[1] * v_in[0] + _matrix[5] * v_in[1] + _matrix[9 ] * v_in[2] + _matrix[13] * v_in[3]);
	v_out[2] = (_matrix[2] * v_in[0] + _matrix[6] * v_in[1] + _matrix[10] * v_in[2] + _matrix[14] * v_in[3]);
	v_out[3] = (_matrix[3] * v_in[0] + _matrix[7] * v_in[1] + _matrix[11] * v_in[2] + _matrix[15] * v_in[3]);
#endif /*NEON*/

	return;
}


TransformerEquirectangular::TransformerEquirectangular(const point_t& focus, float fov_h, float fov_v, float fov_lens, float r):
    _ratio{0.5625f}, // default to 16:9 ratio
    _focus{focus[0], focus[1]},
    _fov_h{fov_h},
    _fov_v{fov_v},
    _r{r/2.0f},
    _fov_lens{fov_lens}
{
}


TransformerEquirectangular::~TransformerEquirectangular()
{
}


void TransformerEquirectangular::Transform(float &x, float &y, uint32_t idx)
{
    const float latitude = _fov_v * (y - 0.5f);
    const float longitude = _fov_h * (x - 0.5f);

    // Point on sphere
    const float sph_x = cosf(latitude) * sinf(longitude);
    const float sph_y = sinf(latitude);
    const float sph_z = cosf(latitude) * cosf(longitude);

    // Fisheye angle and radius
    const float fe_theta = acosf(sph_z);
    const float fe_phi = atan2f(sph_y, sph_x);

    // Use lens original fov to compute radius, which is more accurate for extreme points
    const float fe_r = _r * fe_theta / (_fov_lens / 2.0f);
    
    // Pixel in fisheye space
    // also account for the aspect ratio before mapping into the fisheye image
    x = _focus[0] + fe_r * cosf(fe_phi) * _ratio;
    y = _focus[1] + fe_r * sinf(fe_phi);
}

} // namespace intel_vvp_warp

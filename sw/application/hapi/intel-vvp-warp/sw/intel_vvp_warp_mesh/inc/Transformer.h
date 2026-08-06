/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <cstdint>
#include <vector>
#include "ITransformer.h"
#include "MatrixOperations.h"
#include "WarpConfiguratorTypes.h"

namespace intel_vvp_warp
{

class TransformerAffine : public ITransformer
{
public:
	TransformerAffine(float ratio);
	virtual ~TransformerAffine();
	void Transform(float &x, float &y, uint32_t idx = 0) override;

	void Reset();
	void UpdateRatio(float ratio);
	void SetRotate(float a);
	inline float GetRotate() const
	{
		return _theta;
	}
	void SetHOffset(float v);
	inline float GetHOffset() const
	{
		return _hoff;
	}
	void SetVOffset(float v);
	inline float GetVOffset() const
	{
		return _voff/_ratio;
	}
	void SetZoom(float v);
	inline float GetZoom() const
	{
		return _zoom;
	}

	void SetHSize(float value);
	inline float GetHSize() const
	{
		return _hsize;
	}
	void SetVSize(float value);
	inline float GetVSize() const
	{
		return _vsize/_ratio;
	}

	void SetHMirror(bool v);
	inline bool GetHMirror() const
	{
		return (_hmirr > 0.0f) ? false : true;
	}
	void SetVMirror(bool v);
	inline bool GetVMirror() const
	{
		return (_vmirr > 0.0f) ? false : true;
	}

private:
	void UpdateZoomMatrix();
	void RecalculateMatrix();

	float _ratio;

	// Zoom and Rotate origin
	point_t _origin;

	float _hoff;
	float _voff;
	float _zoom;
	float _hzoom;
	float _vzoom;
	float _theta;

	matrix_t _matrix;
	matrix_t _matrix_translation1;	// Translates to the center of image before rotation and zoom
	matrix_t _matrix_translation2;	// Restores after rotation and zoom
	matrix_t _matrix_rotation;
	matrix_t _matrix_zoom;

	float _hsize;
	float _vsize;

	float _hmirr;
	float _vmirr;
};

//////////////////////
struct arc_conf_t
{
	arc_conf_t();
	bool IsConfigured() const;
	void Reset();

	float _edge[4];
};

// Arcs interpolated using Quadratic Bezier
class TransformerArc: public ITransformer
{
public:
	TransformerArc(const arc_conf_t& arc_conf, bool reverse, bool within_bounding_box = false);
	virtual ~TransformerArc();
	void Transform(float& x, float& y, uint32_t idx = 0) override;

private:
	inline void  _li(const point_t& a, const point_t& b, const float t, point_t& dest)
	{
		dest[0] = a[0] + (b[0]-a[0])*t;
		dest[1] = a[1] + (b[1]-a[1])*t;
	}

	inline void GetPointOnCurve(const point_t& p0, const point_t& p1, const point_t& p2, float t, point_t& p)
	{
		point_t pt01;
		_li(p0, p1, t, pt01);

		point_t pt12;
		_li(p1, p2, t, pt12);

		_li(pt01, pt12, t, p);
	}

	bool _reverse;

	float _mx;

	float _dt;
	float _db;
	float _dl;
	float _dr;

	float _t_offs;
	float _b_offs;
	float _l_offs;
	float _r_offs;

	// Top and bottom arcs
	point_t _pt0;
	point_t _pt1;
	point_t _pt2;
	point_t _pb0;
	point_t _pb1;
	point_t _pb2;

	// Left and right arcs
	point_t _pl0;
	point_t _pl1;
	point_t _pl2;
	point_t _pr0;
	point_t _pr1;
	point_t _pr2;

	point_t _pt;
	point_t _pb;
	point_t _pl;
	point_t _pr;
};


// Radial distortion using Brown-Conrady model
// Forward and inverse variants

class TransformerRadial : public ITransformer
{
public:
	TransformerRadial(float ratio, const point_t& focus, float k1, float k2);
	virtual ~TransformerRadial();
	void Transform(float &x, float &y, uint32_t idx = 0) override;

private:
	float _ratio;
	point_t _focus;
	float _k1;
	float _k2;

};


class TransformerRadialInverse : public ITransformer
{
public:
	TransformerRadialInverse(float ratio, const point_t& focus, float k1, float k2);
	virtual ~TransformerRadialInverse();
	void Transform(float &x, float &y, uint32_t idx = 0) override;

private:
	float _ratio;
	point_t _focus;
	float _k1;
	float _k2;
};



// Note all angles in this transformer are in radians
class TransformerKeystone : public ITransformer
{
public:
	TransformerKeystone(float ratio);
	virtual ~TransformerKeystone();
	void Transform(float &x, float &y, uint32_t idx = 0) override;

	void SetVKeystone(float a);
	void SetHKeystone(float a);
	void SetFOV(float a);
	void SetVAxisOffset(float v);

	inline float GetVKeystone() const
	{
		return _vkey;
	}
	inline float GetHKeystone() const
	{
		return _hkey;
	}
	inline float GetFOV() const
	{
		return _FOV;
	}
	inline float GetVAxisOffset() const
	{
		return _voff;
	}

	void Reset();
	void UpdateRatio(float ratio);

private:

	enum
	{
		ENumCorners = 4,
		ENumPoints = 5
	};

	void UpdateIntersectionPoints();

	float _FOV;					// Projector lens field of view
	float _ratio;
	float _img_width;
	float _img_height;

	float _vkey;				// Vertical compensation angle
	float _hkey;				// Horizontal compensation agnle

	float _voff;				// Optical axis vertical offset

	point_t _DEFAULT_POINTS[ENumCorners];

	// Intersection points
	point_t _ip[ENumCorners];
};


class TransformerPerspective : public ITransformer
{
public:
	TransformerPerspective(const std::vector<float>& matrix);
	TransformerPerspective(const matrix_t& matrix);
	// This one used for wireframe
	TransformerPerspective();
	virtual ~TransformerPerspective();
	void Transform(float &x, float &y, uint32_t idx = 0) override;
	void SetMatrix(const std::vector<float>& matrix);

private:
	void Multiply(const vector4f_t& v_in, vector4f_t& v_out);

private:
	std::vector<float> _matrix;
};


inline void CopyPoints(point_t* dst, const point_t* src, const uint32_t num)
{
	for (uint32_t i = 0; i < num; ++i)
	{
		dst[i][0] = src[i][0];
		dst[i][1] = src[i][1];
	}
}

class TransformerEquirectangular : public ITransformer
{
public:
	TransformerEquirectangular(const point_t& focus, float fov_h, float fov_v, float fov_lens, float r);
	virtual ~TransformerEquirectangular();
	void Transform(float &x, float &y, uint32_t idx = 0) override;

private:
    float _ratio;
	point_t _focus;
	float _fov_h;
	float _fov_v;
    float _r;
    float _fov_lens;
};

} // namespace intel_vvp_warp

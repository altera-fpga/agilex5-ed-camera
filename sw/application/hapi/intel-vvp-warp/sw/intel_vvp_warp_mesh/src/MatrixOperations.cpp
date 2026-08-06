/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "MatrixOperations.h"
#include <cmath>
#include <float.h>	// FLT_EPSILON
#include <algorithm>

namespace intel_vvp_warp
{

void MatrixOperations::DeriveMatrix(float (*sourcePoints)[2], float (*targetPoints)[2], float* matrix)
{
	lu_t lu;
	float m[LU_DIM];

	for(int i = 0; i < 4; ++i)
	{
		float* s = sourcePoints[i];
		float* t = targetPoints[i];

		int idx = 2*i;
		lu._a[idx][0] = s[0];
		lu._a[idx][1] = s[1];
		lu._a[idx][2] = 1;
		lu._a[idx][3] = 0;
		lu._a[idx][4] = 0;
		lu._a[idx][5] = 0;
		lu._a[idx][6] = -s[0] * t[0];
		lu._a[idx][7] = -s[1] * t[0];
		m[idx] = t[0];
		++idx;

		lu._a[idx][0] = 0;
		lu._a[idx][1] = 0;
		lu._a[idx][2] = 0;
		lu._a[idx][3] = s[0];
		lu._a[idx][4] = s[1];
		lu._a[idx][5] = 1;
		lu._a[idx][6] = -s[0] * t[1];
		lu._a[idx][7] = -s[1] * t[1];
		m[idx] = t[1];
	}

	Solve(lu, m);

	matrix[0] = m[0];
	matrix[1] = m[3];
	matrix[2] = 0;
	matrix[3] = m[6];

	matrix[4] = m[1];
	matrix[5] = m[4];
	matrix[6] = 0;
	matrix[7] = m[7];

	matrix[8] = 0;
	matrix[9] = 0;
	matrix[10] = 1;
	matrix[11] = 0;

	matrix[12] = m[2];
	matrix[13] = m[5];
	matrix[14] = 0;
	matrix[15] = 1;
}

void MatrixOperations::MatrixProduct4x4(const matrix_t& a, const matrix_t& b, matrix_t& c)
{
#ifdef NEON
	const float* volatile va = &a[0];
	const float* volatile vb = &b[0];
	float* volatile vc = &c[0];

	// inline asm for multiplication of two 4x4 matrices using neon instructions
	// matrix a loaded into d16-d23 registers
	// matrix b loaded into d0-d7 registers
	// note matrices stored in column major order
	// column products accumulated in q12-q15 registers
	// resulting matrix then stored in column major order in [c]
	__asm__ volatile (
	    "vld1.32  {d16-d19}, [%[matrix_a]]!\n\t"
	    "vld1.32  {d20-d23}, [%[matrix_a]]!\n\t"
	    "vld1.32  {d0-d3}, [%[matrix_b]]!\n\t"
	    "vld1.32  {d4-d7}, [%[matrix_b]]!\n\t"

	    "vmul.f32 q12, q8, d0[0]\n\t"
	    "vmla.f32 q12, q9, d0[1]\n\t"
	    "vmla.f32 q12, q10, d1[0]\n\t"
	    "vmla.f32 q12, q11, d1[1]\n\t"

	    "vmul.f32 q13, q8, d2[0]\n\t"
	    "vmla.f32 q13, q9, d2[1]\n\t"
	    "vmla.f32 q13, q10, d3[0]\n\t"
	    "vmla.f32 q13, q11, d3[1]\n\t"

	    "vmul.f32 q14, q8, d4[0]\n\t"
	    "vmla.f32 q14, q9, d4[1]\n\t"
	    "vmla.f32 q14, q10, d5[0]\n\t"
	    "vmla.f32 q14, q11, d5[1]\n\t"

	    "vmul.f32 q15, q8, d6[0]\n\t"
	    "vmla.f32 q15, q9, d6[1]\n\t"
	    "vmla.f32 q15, q10, d7[0]\n\t"
	    "vmla.f32 q15, q11, d7[1]\n\t"

	    "vst1.32  {d24-d27}, [%[tmp_out]]!\n\t"
	    "vst1.32  {d28-d31}, [%[tmp_out]]!"

	    :
//			:[matrix_a] "r" (&a[0]),[matrix_b] "r" (&b[0]),[tmp_out] "rm" (&c[0])
	    :[matrix_a] "r" (va),[matrix_b] "r" (vb),[tmp_out] "rm" (vc)
	    :"d0","d1","d2","d3","d4","d5","d6","d7","d16","d17","d18","d19","d20","d21","d22","d23","q12","q13","q14","q15","memory"
	);
#else
	c[ 0] = (a[0] * b[ 0] + a[4] * b[ 1] + a[8 ] * b[ 2] + a[12] * b[ 3]);
	c[ 1] = (a[1] * b[ 0] + a[5] * b[ 1] + a[9 ] * b[ 2] + a[13] * b[ 3]);
	c[ 2] = (a[2] * b[ 0] + a[6] * b[ 1] + a[10] * b[ 2] + a[14] * b[ 3]);
	c[ 3] = (a[3] * b[ 0] + a[7] * b[ 1] + a[11] * b[ 2] + a[15] * b[ 3]);

	c[ 4] = (a[0] * b[ 4] + a[4] * b[ 5] + a[8 ] * b[ 6] + a[12] * b[ 7]);
	c[ 5] = (a[1] * b[ 4] + a[5] * b[ 5] + a[9 ] * b[ 6] + a[13] * b[ 7]);
	c[ 6] = (a[2] * b[ 4] + a[6] * b[ 5] + a[10] * b[ 6] + a[14] * b[ 7]);
	c[ 7] = (a[3] * b[ 4] + a[7] * b[ 5] + a[11] * b[ 6] + a[15] * b[ 7]);

	c[ 8] = (a[0] * b[ 8] + a[4] * b[ 9] + a[8 ] * b[10] + a[12] * b[11]);
	c[ 9] = (a[1] * b[ 8] + a[5] * b[ 9] + a[9 ] * b[10] + a[13] * b[11]);
	c[10] = (a[2] * b[ 8] + a[6] * b[ 9] + a[10] * b[10] + a[14] * b[11]);
	c[11] = (a[3] * b[ 8] + a[7] * b[ 9] + a[11] * b[10] + a[15] * b[11]);

	c[12] = (a[0] * b[12] + a[4] * b[13] + a[8 ] * b[14] + a[12] * b[15]);
	c[13] = (a[1] * b[12] + a[5] * b[13] + a[9 ] * b[14] + a[13] * b[15]);
	c[14] = (a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15]);
	c[15] = (a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15]);
#endif /*NEON*/
}


void MatrixOperations::CopyMatrix(matrix_t& dst, const matrix_t& src)
{
	std::copy(&src[0], &src[MATRIX_SIZE], &dst[0]);
}


void MatrixOperations::VectorMatrixColumnProduct(const vector4f_t& v_in, const matrix_t& matrix, vector4f_t& v_out)
{
	v_out[0] = (matrix[0] * v_in[0] + matrix[1] * v_in[1] + matrix[2] * v_in[2] + matrix[3] * v_in[3]);
	v_out[1] = (matrix[4] * v_in[0] + matrix[5] * v_in[1] + matrix[6] * v_in[2] + matrix[7] * v_in[3]);
	v_out[2] = (matrix[8] * v_in[0] + matrix[9] * v_in[1] + matrix[10] * v_in[2] + matrix[11] * v_in[3]);
	v_out[3] = (matrix[12] * v_in[0] + matrix[13] * v_in[1] + matrix[14] * v_in[2] + matrix[15] * v_in[3]);
}


void MatrixOperations::Solve(lu_t& lu, float* m)
{
	LUD(lu);
	LUDSolve(lu, m);
}

// Code below delivers transformation matrix
// from 4 points using LU decomposition
void MatrixOperations::LUD(lu_t& lu)
{
	int k = LU_DIM - 1;

	for(int s = 0; s < LU_DIM; ++s)
	{
		int f = s;
		float* a = lu._a[s];
		float c = fabs(a[s]);

		for(int i = s + 1; i < LU_DIM; ++i)
		{
			float o = fabs(lu._a[i][s]);

			if(c < o)
			{
				c = o;
				f = i;
			}
		}

		lu._p[s] = f;

		if(f != s)
		{
			lu._a[s] = lu._a[f];
			lu._a[f] = a;
			a = lu._a[s];
		}

		float u = a[s];

		for(int r = s + 1; r < LU_DIM; ++r)
		{
			lu._a[r][s] /= u;
			float* l = lu._a[r];

			int i = s + 1;

			while(i < k)
			{
				l[i]-=l[s]*a[i];
				++i;
				l[i]-=l[s]*a[i];
				++i;
			}

			if(i == k)
				l[i] -= l[s] * a[i];
		}
	}
}


void MatrixOperations::LUDSolve(lu_t& lu, float* m)
{
	for(int i = 0; i < LU_DIM; ++i)
	{
		int f = lu._p[i];

		if(lu._p[i] != i)
		{
			float h = m[i];
			m[i] = m[f];
			m[f] = h;
		}

		for(int j = 0; j < i ; ++j)
			m[i] -= m[j] * lu._a[i][j];
	}

	for(int i = LU_DIM - 1; i >= 0; --i)
	{
		for(int j = i + 1; j < LU_DIM; ++j)
			m[i] -= m[j] * lu._a[i][j];

		m[i] /= lu._a[i][i];
	}
}


void MatrixOperations::RotationMatrix(const float yaw, const float pitch, const float roll, matrix_t& m)
{
	matrix_t matrix_rx = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	matrix_t matrix_ry = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
	matrix_t matrix_rz = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

	float cos_theta = 0.0f;
	float sin_theta = 0.0f;

	// Yaw
	cos_theta = cosf(yaw);
	sin_theta = sinf(yaw);

	matrix_rz[0] = cos_theta;
	matrix_rz[1] = -sin_theta;
	matrix_rz[4] = sin_theta;
	matrix_rz[5] = cos_theta;

	// Pitch
	cos_theta = cosf(pitch);
	sin_theta = sinf(pitch);

	matrix_ry[0] = cos_theta;
	matrix_ry[2] = sin_theta;
	matrix_ry[8] = -sin_theta;
	matrix_ry[10] = cos_theta;

	// Roll
	cos_theta = cosf(roll);
	sin_theta = sinf(roll);

	matrix_rx[5] = cos_theta;
	matrix_rx[6] = -sin_theta;
	matrix_rx[9] = sin_theta;
	matrix_rx[10] = cos_theta;

	matrix_t matrix_tmp;
	MatrixProduct4x4(matrix_rx, matrix_ry, matrix_tmp);
	MatrixProduct4x4(matrix_tmp, matrix_rz, m);
}


void MatrixOperations::VectorCrossProduct(const vector4f_t& v0, const vector4f_t& v1, vector4f_t& v)
{
	v[0] = (v0[1] * v1[2]) - (v0[2] * v1[1]);
	v[1] = (v0[2] * v1[0]) - (v0[0] * v1[2]);
	v[2] = (v0[0] * v1[1]) - (v0[1] * v1[0]);
	v[3] = 0.0f;
}


float MatrixOperations::VectorDotProduct(const vector4f_t& v0, const vector4f_t& v1)
{
	return v0[0] * v1[0] + v0[1] * v1[1] + v0[2] * v1[2];
}


plane_t MatrixOperations::PlaneFromThreePoints(const vector4f_t& p0, const vector4f_t& p1, const vector4f_t& p2)
{
	plane_t plane;

	vector4f_t v0;
	v0[0] = p1[0] - p0[0];
	v0[1] = p1[1] - p0[1];
	v0[2] = p1[2] - p0[2];
	v0[3] = p1[3] - p0[3];

	vector4f_t v1;
	v1[0] = p2[0] - p0[0];
	v1[1] = p2[1] - p0[1];
	v1[2] = p2[2] - p0[2];
	v1[3] = p2[3] - p0[3];

	VectorCrossProduct(v0, v1, plane._n);

	plane._p[0] = p0[0];
	plane._p[1] = p0[1];
	plane._p[2] = p0[2];
	plane._p[3] = p0[3];

	return plane;
}


void MatrixOperations::PlaneLineIntersection(const plane_t& plane, const vector4f_t& p0, const vector4f_t& p1, vector4f_t& p)
{
	vector4f_t l;
	l[0] = p1[0] - p0[0];
	l[1] = p1[1] - p0[1];
	l[2] = p1[2] - p0[2];
	l[3] = p1[3] - p0[3];

	auto float_not_zero = [](const float v)->bool
	{
		return (fabs(v - 0.0f) > FLT_EPSILON);
	};

	float prod1 = VectorDotProduct(l, plane._n);

	if (float_not_zero(prod1))
	{
		vector4f_t diff;
		diff[0] = p0[0] - plane._p[0];
		diff[1] = p0[1] - plane._p[1];
		diff[2] = p0[2] - plane._p[2];
		diff[3] = p0[3] - plane._p[3];

		float prod2 = VectorDotProduct(diff, plane._n);
		float prod3 = prod2 / prod1;

		p[0] = p0[0] - l[0] * prod3;
		p[1] = p0[1] - l[1] * prod3;
		p[2] = p0[2] - l[2] * prod3;
		p[3] = p0[3] - l[3] * prod3;
	}
}

} // namespace intel_vvp_warp

/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

namespace intel_vvp_warp
{

typedef float vector4f_t[4];

static constexpr unsigned int MATRIX_SIZE = 16;
typedef float matrix_t[MATRIX_SIZE];

struct plane_t
{
	vector4f_t _n; // Normal
	vector4f_t _p;
};

class MatrixOperations
{
public:
	static void DeriveMatrix(float (*sourcePoints)[2], float (*targetPoints)[2], float* matrix);
	static void MatrixProduct4x4(const matrix_t& a, const matrix_t& b, matrix_t& c);
	static void CopyMatrix(matrix_t& dst, const matrix_t& src);

	static void RotationMatrix(const float yaw, const float pitch, const float roll, matrix_t& m);
	static void VectorMatrixColumnProduct(const vector4f_t& v_in, const matrix_t& matrix, vector4f_t& v_out);

	static void VectorCrossProduct(const vector4f_t& v0, const vector4f_t& v1, vector4f_t& v);
	static float VectorDotProduct(const vector4f_t& v0, const vector4f_t& v1);

	static plane_t PlaneFromThreePoints(const vector4f_t& p0, const vector4f_t& p1, const vector4f_t& p2);
	static void PlaneLineIntersection(const plane_t& plane, const vector4f_t& p0, const vector4f_t& p1, vector4f_t& p);

private:
	static const int LU_DIM = 8;

	struct lu_t
	{
		float** _a;
		int _p[LU_DIM];

		lu_t(const lu_t&) = delete;
		lu_t& operator=(const lu_t&) = delete;

		lu_t():
			_a(new float* [LU_DIM])
		{
			for(int i = 0; i < LU_DIM; ++i)
				_a[i] = new float[LU_DIM];
		}

		~lu_t()
		{
			for(int i = 0; i < LU_DIM; ++i)
				delete[] _a[i];

			delete[] _a;
		}
	};

	static void Solve(lu_t& lu, float* m);
	static void LUD(lu_t& lu);
	static void LUDSolve(lu_t& lu, float* m);
};

} // namespace intel_vvp_warp

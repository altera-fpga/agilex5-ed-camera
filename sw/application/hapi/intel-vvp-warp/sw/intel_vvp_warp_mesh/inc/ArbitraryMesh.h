/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <vector>
#include "Triangle.h"
#include "WarpMesh.h"

namespace intel_vvp_warp
{

class ArbitraryMesh
{
	friend class WarpConfigurator;

public:
	using TPoint = Point<float>;

	ArbitraryMesh(uint32_t vKnots, uint32_t hKnots, std::vector<TPoint> inputKnotPoints);
	virtual ~ArbitraryMesh();

	void ToReverse(uint32_t input_width, uint32_t input_height, uint32_t output_width, uint32_t output_height, WarpMesh& warpMesh);

private:
	void ExtrapolateKnots(uint32_t& vKnots, uint32_t& hKnots, std::vector<TPoint>& knotPoints);
	void ApplySpline(TPoint** pts, uint32_t numPoints, uint32_t numSegments);
	void Translate(uint32_t input_width, uint32_t input_height, uint32_t output_width, uint32_t output_height, const uint32_t mesh_frac_prec);
	bool ToOutput(WarpMesh& mesh);
	bool IsSegmentOutsideScreen(TPoint p1, TPoint p2);
	const std::vector<TPoint>& GetMeshPoints()
	{
		return _mesh;
	}

	void Debug(const char* filename);

	uint32_t _extraKnots;	// Number of extrapolated knots (same on each side)

	uint32_t _vPoints;
	uint32_t _hPoints;
	std::vector<TPoint> _mesh;

	using TMeshLines = std::vector<TPoint*>;
	static constexpr uint32_t NUM_SEGMENTS	= 16;

	TMeshLines _vLines;
	TMeshLines _hLines;

	struct LineWrapper
	{
		LineWrapper(TMeshLines& lines, const std::size_t line_size):
			_lines(lines), _line_size(line_size)
		{}

		inline TPoint** operator[](std::size_t i)
		{
			return &(_lines[i * _line_size]);
		}

	private:
		TMeshLines& _lines;
		const std::size_t _line_size;
	};

	std::vector<Triangle<int> > _triangles;
	std::vector<Triangle<int> > _triangles_orig;
};

} // namespace intel_vvp_warp

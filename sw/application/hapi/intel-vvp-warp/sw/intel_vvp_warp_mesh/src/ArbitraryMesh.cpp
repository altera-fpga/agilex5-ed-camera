/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <deque>
#include "ArbitraryMesh.h"

namespace intel_vvp_warp
{


ArbitraryMesh::ArbitraryMesh(uint32_t vKnots, uint32_t hKnots, std::vector<TPoint> inputKnotPoints):
	_extraKnots{0}
{
	ExtrapolateKnots(vKnots, hKnots, inputKnotPoints);

	uint32_t vIntervals = vKnots - 1;
	uint32_t vSegments = NUM_SEGMENTS;
	_vPoints = vSegments * vIntervals + 1;

	uint32_t hIntervals = hKnots - 1;
	uint32_t hSegments = NUM_SEGMENTS;
	_hPoints = hSegments * hIntervals + 1;

	float v_step = 1.0f / static_cast<float>(_vPoints - 1);
	float h_step = 1.0f / static_cast<float>(_hPoints - 1);

	const std::size_t totalPoints = _vPoints * _hPoints;
	_mesh.resize(totalPoints);
	_vLines.resize(totalPoints);
	_hLines.resize(totalPoints);

	LineWrapper vLines{ _vLines, _vPoints };
	LineWrapper hLines{ _hLines, _hPoints };

	for (uint32_t i = 0; i < _vPoints; ++i)
	{
		for (uint32_t j = 0; j < _hPoints; ++j)
		{
			float x = static_cast<float>(j) * h_step;
			float y = static_cast<float>(i) * v_step;

			TPoint& thisPoint = _mesh[i * _hPoints + j];
			thisPoint._x = x;
			thisPoint._y = y;

			hLines[i][j] = &thisPoint;
			vLines[j][i] = &thisPoint;
		}
	}

	///////// Fill in existing knots //////////
	std::size_t knotIdx = 0;
	for (uint32_t i = 0; i < _vPoints; i += vSegments)
	{
		for (uint32_t j = 0; j < _hPoints; j += hSegments)
		{
			const TPoint& p = inputKnotPoints.at(knotIdx);
			*(hLines[i][j]) = p;
			++knotIdx;
		}
	}

	/////////// Apply splines /////////
	for (uint32_t i = 0; i < _vPoints; ++i)
		ApplySpline(hLines[i], _hPoints, hSegments);

	for (uint32_t i = 0; i < _hPoints; ++i)
		ApplySpline(vLines[i], _vPoints, vSegments);
}


ArbitraryMesh::~ArbitraryMesh()
{
}


void ArbitraryMesh::ExtrapolateKnots(uint32_t& vKnots, uint32_t& hKnots, std::vector<TPoint>& knotPoints)
{
	// Check existing knots cover entire frame
	bool frameCovered = false;

    // Limit number of iterations to protect
    // against random user input
    uint32_t iter_limit = 8;

	while (!frameCovered && iter_limit)
	{
		frameCovered = true;

		// Top and bottom
		uint32_t idx1 = 0;
		uint32_t idx2 = (vKnots - 1) * hKnots;

		for (uint32_t i = 0; i < (hKnots - 1); ++i)
		{
			const auto& point1 = knotPoints[idx1];
			++idx1;
			const auto& point2 = knotPoints[idx1];
			const auto& point3 = knotPoints[idx2];
			++idx2;
			const auto& point4 = knotPoints[idx2];

			if (IsSegmentOutsideScreen(point1, point2))
				if (IsSegmentOutsideScreen(point3, point4))
					continue;

			frameCovered = false;
			break;
		}

		// Left and right
		if (frameCovered)
		{
			uint32_t idx1 = 0;
			uint32_t idx2 = hKnots - 1;

			for (uint32_t i = 0; i < (vKnots - 1); ++i)
			{
				const auto& point1 = knotPoints[idx1];
				idx1 += hKnots;
				const auto& point2 = knotPoints[idx1];
				const auto& point3 = knotPoints[idx2];
				idx2 += hKnots;
				const auto& point4 = knotPoints[idx2];

				if (IsSegmentOutsideScreen(point1, point2))
					if (IsSegmentOutsideScreen(point3, point4))
						continue;

				frameCovered = false;
				break;
			}
		}

		// If frame not covered add extra column/row on each side
		if (!frameCovered)
		{
			{
				// Extend knot grid by adding a new row all around
				uint32_t vPointsNew = vKnots + 2;
				uint32_t hPointsNew = hKnots + 2;
				std::vector<TPoint> knotPointsNew(vPointsNew * hPointsNew);

				uint32_t src_idx = 0;

				// Copy old knots
				for (uint32_t v = 1; v < (vPointsNew - 1); ++v)
				{
					for (uint32_t h = 1; h < (hPointsNew - 1); ++h)
					{
						uint32_t dst_idx = v * hPointsNew + h;
						knotPointsNew[dst_idx] = knotPoints[src_idx];
						++src_idx;
					}
				}

				knotPoints = std::move(knotPointsNew);
				vKnots = vPointsNew;
				hKnots = hPointsNew;
			}

			// Generate values for added knots linearly extrapolating existing

			auto extrapolate_knot = [](TPoint& p, const TPoint& p1, const TPoint& p2) -> void
			{
				p._x = p1._x - (p2._x - p1._x);
				p._y = p1._y - (p2._y - p1._y);
			};

			// Top and bottom
			uint32_t dst1 = 1;
			uint32_t dst2 = dst1 + ((vKnots - 1) * hKnots);

			for (uint32_t h = 1; h < (hKnots - 1); ++h)
			{
				extrapolate_knot(knotPoints[dst1], knotPoints[dst1 + hKnots], knotPoints[dst1 + hKnots + hKnots]);
				extrapolate_knot(knotPoints[dst2], knotPoints[dst2 - hKnots], knotPoints[dst2 - hKnots - hKnots]);
				++dst1;
				++dst2;
			}           

			// Left and right
			dst1 = hKnots;
			dst2 = dst1 + hKnots - 1;

			for (uint32_t v = 1; v < (vKnots - 1); ++v)
			{
				extrapolate_knot(knotPoints[dst1], knotPoints[dst1 + 1], knotPoints[dst1 + 2]);
				extrapolate_knot(knotPoints[dst2], knotPoints[dst2 - 1], knotPoints[dst2 - 2]);
				dst1 += hKnots;
				dst2 += hKnots;
			}

			// Corner points
			// top left
			extrapolate_knot(knotPoints[0], knotPoints[hKnots + 1], knotPoints[2 * hKnots + 2]);

			// top right
			extrapolate_knot(knotPoints[hKnots - 1], knotPoints[2 * hKnots - 2], knotPoints[3 * hKnots - 3]);

			// bottom left
			extrapolate_knot(knotPoints[(vKnots - 1) * hKnots], knotPoints[(vKnots - 2) * hKnots + 1], knotPoints[(vKnots - 3) * hKnots + 2]);

			// bottom right
			extrapolate_knot(knotPoints[vKnots * hKnots - 1], knotPoints[(vKnots - 1) * hKnots - 2], knotPoints[(vKnots - 2) * hKnots - 3]);

			++_extraKnots;	// 1 knot added in all dimensions
		}

        --iter_limit;
	} // while(!frameCovered)
}


void ArbitraryMesh::ApplySpline(TPoint** pts, uint32_t numPoints, uint32_t numSegments)
{
	float tension = 0.5f;

	std::deque<TPoint*> knots;

	for (uint32_t i = 0; i < numPoints; i += numSegments)
		knots.push_back(pts[i]);

	// Fake points required to calculate
	// tangents for the 1st and last knots
	TPoint fakeFront;
	TPoint fakeEnd;

	// Duplicate first point to befinning, end point to end
	std::size_t numKnots = knots.size();
	fakeFront._x = knots[0]->_x - (knots[1]->_x - knots[0]->_x);
	fakeFront._y = knots[0]->_y - (knots[1]->_y - knots[0]->_y);
	fakeEnd._x = knots[numKnots - 1]->_x + (knots[numKnots - 1]->_x - knots[numKnots - 2]->_x);
	fakeEnd._y = knots[numKnots - 1]->_y + (knots[numKnots - 1]->_y - knots[numKnots - 2]->_y);

	knots.push_front(&fakeFront);
	knots.push_back(&fakeEnd);

	// Iterate through each segment + the point before and after

	for (uint32_t i = 1; i < (uint32_t)(knots.size() - 2); ++i)
	{
		// calc tension vectors
		float t1x = (knots[i + 1]->_x - knots[i - 1]->_x) * tension;
		float t2x = (knots[i + 2]->_x - knots[i]->_x) * tension;
		float t1y = (knots[i + 1]->_y - knots[i - 1]->_y) * tension;
		float t2y = (knots[i + 2]->_y - knots[i]->_y) * tension;

		for (uint32_t t = 0; t <= numSegments; ++t)
		{
			// Step
			float st = (float)t / (float)numSegments;
			float st_pow2 = st * st;
			float st_pow3 = st_pow2 * st;

			// Cardinals
			float c1 = 2.0f * st_pow3 - 3.0f * st_pow2 + 1.0f;
			float c2 = -(2.0f * st_pow3) + 3.0f * st_pow2;
			float c3 = st_pow3 - 2.0f * st_pow2 + st;
			float c4 = st_pow3 - st_pow2;

			// x and y coordinates
			float x = c1 * knots[i]->_x + c2 * knots[i + 1]->_x + c3 * t1x + c4 * t2x;
			float y = c1 * knots[i]->_y + c2 * knots[i + 1]->_y + c3 * t1y + c4 * t2y;

			uint32_t idx = ((i - 1) * numSegments) + t;

			pts[idx]->_x = x;
			pts[idx]->_y = y;
		}
	}
	return;
}


void ArbitraryMesh::Translate(uint32_t input_width, uint32_t input_height, uint32_t output_width, uint32_t output_height, const uint32_t mesh_frac_prec)
{
	_triangles.clear();
	_triangles_orig.clear();

	// 4 fraction bits added straight away
	// Otherwise we may lose precision
	// There are NUM_SEGMENTS extra points extrapolated on each side
	// to make sure entire screen is covered by the mesh
	const int default_section_width = (input_width << mesh_frac_prec) / (_hPoints - (2 * _extraKnots * NUM_SEGMENTS) - 1);
	const int default_section_height = (input_height << mesh_frac_prec) / (_vPoints - (2 * _extraKnots * NUM_SEGMENTS) - 1);
	const int default_section_hoffset = default_section_width * _extraKnots * NUM_SEGMENTS;
	const int default_section_voffset = default_section_height * _extraKnots *  NUM_SEGMENTS;

	LineWrapper hLines{ _hLines, _hPoints };

	for (uint32_t v = 0; v < (_vPoints - 1); ++v)
	{
		for (uint32_t h = 0; h < (_hPoints - 1); ++h)
		{
			const TPoint* p1 = hLines[v][h];
			const TPoint* p2 = hLines[v][h + 1];
			const TPoint* p3 = hLines[v + 1][h];
			const TPoint* p4 = hLines[v + 1][h + 1];

			// 4 fractional bits
			float widthf = static_cast<float>(output_width << mesh_frac_prec);
			float heightf = static_cast<float>(output_height << mesh_frac_prec);

			// 4 fraction bits
			int x1 = (int)(p1->_x * widthf);
			int y1 = (int)(p1->_y * heightf);

			int x2 = (int)(p2->_x * widthf);
			int y2 = (int)(p2->_y * heightf);

			int x3 = (int)(p3->_x * widthf);
			int y3 = (int)(p3->_y * heightf);

			int x4 = (int)(p4->_x * widthf);
			int y4 = (int)(p4->_y * heightf);

			_triangles.push_back(
			    Triangle<int>(
			        Point<int>(x1, y1),
			        Point<int>(x4, y4),
			        Point<int>(x3, y3)
			    )
			);

			_triangles.push_back(
			    Triangle<int>(
			        Point<int>(x1, y1),
			        Point<int>(x2, y2),
			        Point<int>(x4, y4)
			    )
			);

			x1 = h * default_section_width - default_section_hoffset;
			x2 = x1 + default_section_width;
			y1 = v * default_section_height - default_section_voffset;
			y2 = y1 + default_section_height;


			_triangles_orig.push_back(
			    Triangle<int>(
			        Point<int>(x1, y1),
			        Point<int>(x2, y2),
			        Point<int>(x1, y2)
			    )
			);

			_triangles_orig.push_back(
			    Triangle<int>(
			        Point<int>(x1, y1),
			        Point<int>(x2, y1),
			        Point<int>(x2, y2)
			    )
			);
		}
	}   
}


bool ArbitraryMesh::ToOutput(WarpMesh& mesh)
{
    const uint32_t mesh_fract_bits = mesh.GetFractBits();

	uint32_t width = ((mesh.GetHNodes() - 1) * mesh.GetStep()) << mesh_fract_bits;
	uint32_t height = ((mesh.GetVNodes() - 1) * mesh.GetStep()) << mesh_fract_bits;

	for (std::size_t i = 0; i < _triangles.size(); ++i)
	{
		const Triangle<int>& t = _triangles.at(i);
		t.InterpolateEx(_triangles_orig.at(i), mesh, width, height);
	}

	return true;
}


void ArbitraryMesh::ToReverse(uint32_t input_width, uint32_t input_height, uint32_t output_width, uint32_t output_height, WarpMesh& warpMesh)
{
	Translate(input_width, input_height, output_width, output_height, (warpMesh.GetStep() == 16 ? 3 : 4));
	ToOutput(warpMesh);
}


// Uses Cohen-Sutherland algorithm to find if a line is outside the screen edge
bool ArbitraryMesh::IsSegmentOutsideScreen(TPoint p1, TPoint p2)
{
	enum ECode
	{
		INSIDE = 0,	// 0000
		LEFT = 1,	// 0001
		RIGHT = 2,	// 0010
		BOTTOM = 4, // 0100
		TOP = 8		// 1000
	};

	auto get_point_code = [](const TPoint* p)->uint32_t
	{
		uint32_t code = INSIDE;

		if (p->_x < 0.0f)
			code |= LEFT;
		else if (p->_x > 1.0f)
			code |= RIGHT;
		if (p->_y < 0.0f)
			code |= BOTTOM;
		else if (p->_y > 1.0f)
			code |= TOP;

		return code;
	};

	bool segment_outside = false;

	uint32_t code1 = get_point_code(&p1);
	uint32_t code2 = get_point_code(&p2);

	while (true)
	{
		// Segment inside screen - exit
		if (0 == (code1 | code2))
			break;
		// Segment outside screen - exit
		else if (code1 & code2)
		{
			segment_outside = true;
			break;
		}
		// Segment may cross the screen
		else
		{
			TPoint p{0.0f, 0.0f};

			// At least one endpoint is outside the screen;
			uint32_t codeOut = code1 ? code1 : code2;

			// Find intersection with screen edges;
			if (codeOut & TOP) // point is above the clip rectangle
			{
				p._x = p1._x + (p2._x - p1._x) * (1.0f - p1._y) / (p2._y - p1._y);
				p._y = 1.0f;
			}
			else if (codeOut & BOTTOM)  // point is below the clip rectangle
			{
				p._x = p1._x + (p2._x - p1._x) * (0.0f - p1._y) / (p2._y - p1._y);
				p._y = 0.0f;
			}
			else if (codeOut & RIGHT)    // point is to the right of clip rectangle
			{
				p._y = p1._y + (p2._y - p1._y) * (1.0f - p1._x) / (p2._x - p1._x);
				p._x = 1.0f;
			}
			else if (codeOut & LEFT)     // point is to the left of clip rectangle
			{
				p._y = p1._y + (p2._y - p1._y) * (0.0f - p1._x) / (p2._x - p1._x);
				p._x = 0.0f;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (codeOut == code1)
			{
				p1 = p;
				code1 = get_point_code(&p1);
			}
			else
			{
				p2 = p;
				code2 = get_point_code(&p2);
			}
		}
	}

	return segment_outside;
};


void ArbitraryMesh::Debug(const char* filename)
{
	const std::string html_data_filename(filename);
	std::ofstream html_data_out(html_data_filename.c_str(), std::ios::out);

	if (html_data_out.is_open())
	{
		const uint32_t width = 1024;
		const uint32_t height = 576;

		LineWrapper hLines{ _hLines, _hPoints };

		const uint32_t border = 100;

		html_data_out << "<html><body><svg width=\"960\" height=\"540\" style=\"border:1px solid red\" viewBox=\"0,0,";
		html_data_out << width + 2 * border << ",";
		html_data_out << height + 2 * border << "\"><g transform=\"translate(";
		html_data_out << border << "," << border << ")\">" << std::endl;

		// Screen
		html_data_out << "<rect x=\"0\" y=\"0\" width=\"" << width << "\" height=\"" << height << "\" stroke=\"green\" fill=\"none\" stroke-width=\"2\"/>" << std::endl;

		for (uint32_t i = 0; i < (_vPoints - 1); ++i)
		{
			for (uint32_t j = 0; j < (_hPoints - 1); ++j)
			{
				TPoint* p0 = hLines[i][j];
				TPoint* p1 = hLines[i][j + 1];
				TPoint* p2 = hLines[i + 1][j];
				TPoint* p3 = hLines[i + 1][j + 1];

				html_data_out << "<path d=\"M" << (p0->_x * width) << "," << (p0->_y * height);
				html_data_out << "L" << (p1->_x * width) << "," << (p1->_y * height);
				html_data_out << "L" << (p3->_x * width) << "," << (p3->_y * height);
				html_data_out << "Z\" fill=\"transparent\" stroke=\"black\"></path>" << std::endl;

				html_data_out << "<path d=\"M" << (p0->_x * width) << "," << (p0->_y * height);
				html_data_out << "L" << (p3->_x * width) << "," << (p3->_y * height);
				html_data_out << "L" << (p2->_x * width) << "," << (p2->_y * height);
				html_data_out << "Z\" fill=\"transparent\" stroke=\"black\"></path>" << std::endl;
			}
		}

		for (uint32_t i = 0; i < _vPoints; i += NUM_SEGMENTS)
		{
			for (uint32_t j = 0; j < _hPoints; j += NUM_SEGMENTS)
			{
				TPoint* p0 = hLines[i][j];
				html_data_out << "<circle cx=\"" << p0->_x * width << "\"cy=\"" << p0->_y * height << "\" r=\"8\" stroke=\"black\" stroke-width=\"2\" fill=\"none\" />" << std::endl;
			}
		}

		html_data_out << "</g></svg></body></html>" << std::endl;
		html_data_out.close();
	}
	else
		std::cerr << "Error opening " << html_data_filename << std::endl;
}

} // namespace intel_vvp_warp

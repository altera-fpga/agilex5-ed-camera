/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpConfigurator.h"
#include "ArbitraryMesh.h"
#include "WarpConfiguratorUtils.h"
#include <algorithm>
#include <iostream>
#include <cfloat>
#include <cstring>

#ifndef ALT_SINGLE_THREADED
#include <future>
#endif /* ALT_SINGLE_THREADED */


namespace intel_vvp_warp
{

namespace
{
constexpr uint32_t MAX_WIDTH = 8192;
constexpr uint32_t MAX_HEIGHT = 8192;
constexpr uint32_t DEFAULT_WIDTH = 3840;
constexpr uint32_t DEFAULT_HEIGHT = 2160;

#define DEFAULT_PERSPECTIVE_POINTS  {{0.0f, 0.0f},{1.0f, 0.0f},{0.0f, 1.0f},{1.0f, 1.0f}}
constexpr point_t DEFAULT_CORNERS[4] = DEFAULT_PERSPECTIVE_POINTS;
constexpr uint32_t DEFAULT_ARBITRARY_SIZE = 3;
}


WarpConfigurator::WarpConfigurator(const WarpHwContextPtr& hw_ctx):
    _input_res{DEFAULT_WIDTH, DEFAULT_HEIGHT},
    _output_res{DEFAULT_WIDTH, DEFAULT_HEIGHT},
    _kt{0.5625f},    // default to 16:9 ratio
    _at{0.5625f},
    _maintain_ratio{false},
    _hw_ctx{hw_ctx},
    _fe_transform{EFisheyeTransform::EquiRectangular},
    _fe_lens_fov{degrees_to_radians(180.0f)},
    _fe_r{0.0f},
    _ftp_latitude_min{0.0f},
    _ftp_latitude_max{degrees_to_radians(90.0f)},
    _ftp_longitude{0.0f},
    _ftp_fov_h{degrees_to_radians(90.0f)},
    _ftp_insideout{true},
    _equirect_fov_h{degrees_to_radians(180.0f)},
    _equirect_fov_v{degrees_to_radians(180.0f)}
{
    Reset();
}


WarpConfigurator::~WarpConfigurator()
{
}


void WarpConfigurator::SetHSize(float pixels)
{
    _at.SetHSize(get_norm_val(pixels, static_cast<float>(_output_res.first)));
}


float WarpConfigurator::GetHSize() const
{
    return get_denorm_val(_at.GetHSize(), static_cast<float>(_output_res.first));
}


void WarpConfigurator::SetVSize(float pixels)
{
    _at.SetVSize(get_norm_val(pixels, static_cast<float>(_output_res.second)));
}


float WarpConfigurator::GetVSize() const
{
    return get_denorm_val(_at.GetVSize(), static_cast<float>(_output_res.second));
}


void WarpConfigurator::SetHOffset(float pixels)
{
    _at.SetHOffset(get_norm_val(pixels, static_cast<float>(_output_res.first)));
}


float WarpConfigurator::GetHOffset() const
{
    return get_denorm_val(_at.GetHOffset(), static_cast<float>(_output_res.first));
}


void WarpConfigurator::SetVOffset(float pixels)
{
    _at.SetVOffset(get_norm_val(pixels, static_cast<float>(_output_res.second)));
}


float WarpConfigurator::GetVOffset() const
{
    return get_denorm_val(_at.GetVOffset(), static_cast<float>(_output_res.second));
}


void WarpConfigurator::SetHMirror(bool val)
{
    _at.SetHMirror(val);
}


bool WarpConfigurator::GetHMirror() const
{
    return _at.GetHMirror();
}


void WarpConfigurator::SetVMirror(bool val)
{
    _at.SetVMirror(val);
}


bool WarpConfigurator::GetVMirror() const
{
    return _at.GetVMirror();
}


void WarpConfigurator::SetHKeystone(float angle)
{
    _kt.SetHKeystone(degrees_to_radians(angle));
}


float WarpConfigurator::GetHKeystone() const
{
    return radians_to_degrees(_kt.GetHKeystone());
}


void WarpConfigurator::SetVKeystone(float angle)
{
    _kt.SetVKeystone(degrees_to_radians(angle));
}


float WarpConfigurator::GetVKeystone() const
{
    return radians_to_degrees(_kt.GetVKeystone());
}


void WarpConfigurator::SetZoom(float zoom)
{
    _at.SetZoom(zoom);
}


float WarpConfigurator::GetZoom() const
{
    return _at.GetZoom();
}


void WarpConfigurator::SetFOV(float angle)
{
    _kt.SetFOV(degrees_to_radians(angle));
}


float WarpConfigurator::GetFOV() const
{
    return radians_to_degrees(_kt.GetFOV());
}


void WarpConfigurator::SetVAxisOffset(float v)
{
    _kt.SetVAxisOffset(v);
}


float WarpConfigurator::GetVAxisOffset() const
{
    return _kt.GetVAxisOffset();
}


void WarpConfigurator::SetRotate(float angle)
{
    _at.SetRotate(degrees_to_radians(angle));
}


float WarpConfigurator::GetRotate() const
{
    return radians_to_degrees(_at.GetRotate());
}


void WarpConfigurator::SetArc(const EEdgeId& edge, float val)
{
    _arc_conf._edge[edge] = val;
}


float WarpConfigurator::GetArc(const EEdgeId& edge) const
{
    return _arc_conf._edge[edge];
}


std::pair<float, float> WarpConfigurator::GetCorner(const ECornerId& corner) const
{
    float x = 0.0f, y = 0.0f;

    if(corner < (sizeof(_corners)/sizeof(point_t)))
    {
        const auto [xn, yn] = GetCornerNorm(corner);

        x = get_denorm_val(xn, static_cast<float>(_output_res.first));
        y = get_denorm_val(yn, static_cast<float>(_output_res.second));
    }

    return std::make_pair(x, y);
}


std::pair<float, float> WarpConfigurator::GetCornerNorm(const ECornerId& corner) const
{
    float x = 0.0f, y = 0.0f;

    if(corner < (sizeof(_corners)/sizeof(point_t)))
    {
        const point_t& point = _corners[corner];

        x = point[0];
        y = point[1];
    }

    return std::make_pair(x, y);
}


void WarpConfigurator::SetCorner(const ECornerId& corner, float pixel_x, float pixel_y)
{
    SetCornerNorm(corner, get_norm_val(pixel_x, static_cast<float>(_output_res.first)), get_norm_val(pixel_y, static_cast<float>(_output_res.second)));
}


void WarpConfigurator::SetCornerNorm(const ECornerId& corner, float x, float y)
{
    if(corner >= (sizeof(_corners)/sizeof(point_t)))
        return;

    point_t& point = _corners[corner];

    point[0] = x;
    point[1] = y;
}


void WarpConfigurator::SetMaintainRatio(bool val)
{
    _maintain_ratio = val;
}


void WarpConfigurator::Reset()
{
    _kt.Reset();
    _at.Reset();
    _shrink_to_fit = false;
    _arc_conf.Reset();
    ResetCorners();
    _radial._focus[0] = 0.5f;
    _radial._focus[1] = 0.5f;
    _radial._k1 = 0.0f;
    _radial._k2 = 0.0f;
    SetArbitraryKnotsNum(DEFAULT_ARBITRARY_SIZE);
    _fe_transform = EFisheyeTransform::None;
}


void WarpConfigurator::ResetCorners()
{
    CopyPoints(_corners, DEFAULT_CORNERS, ETotalCorners);
}


void WarpConfigurator::TransformPerspectivePoints(point_t* p, std::size_t n, bool use_corners)
{
    if(use_corners)
    {
        CopyPoints(p, _corners, ETotalCorners);
    }
    else
    {
        for(std::size_t j=0; j<n; ++j)
        {
            _kt.Transform(p[j][0], p[j][1], j);
            _at.Transform(p[j][0], p[j][1], j);
        }
    }
}


std::vector<float> WarpConfigurator::GetPerspectiveMatrix(bool use_corners)
{
    point_t source_points[4] = DEFAULT_PERSPECTIVE_POINTS;
    point_t target_points[4] = DEFAULT_PERSPECTIVE_POINTS;

    TransformPerspectivePoints(target_points, 4, use_corners);

    matrix_t matrix;
    MatrixOperations::DeriveMatrix(source_points, target_points, matrix);
    std::vector<float> m(&matrix[0], &matrix[16]);

    return m;
}


std::vector<float> WarpConfigurator::GetInversePerspectiveMatrix(bool use_corners)
{
    point_t source_points[4] = DEFAULT_PERSPECTIVE_POINTS;
    point_t target_points[4] = DEFAULT_PERSPECTIVE_POINTS;

    TransformPerspectivePoints(target_points, 4, use_corners);

    matrix_t matrix;
    // Swap source and target points to get reverse transform
    MatrixOperations::DeriveMatrix(target_points, source_points, matrix);
    std::vector<float> m(&matrix[0], &matrix[16]);

    return m;
}


std::vector<float> WarpConfigurator::GetPrescaleMatrix()
{
    const float output_ratio = static_cast<float>(_output_res.first) / static_cast<float>(_output_res.second);
    const float input_ratio = static_cast<float>(_input_res.first) / static_cast<float>(_input_res.second);

    float width = 1.0f;
    float height = 1.0f;

    if(input_ratio > output_ratio)
        height = height * (output_ratio/input_ratio);
    else
        width = width * (input_ratio/output_ratio);

    float x1 = (1.0f - width) / 2.0f;
    float y1 = (1.0f - height) / 2.0f;
    float x2 = x1 + width;
    float y2 = y1 + height;

    point_t source_points[4] = {{x1, y1}, {x2, y1}, {x1, y2}, {x2, y2}};
    point_t target_points[4] = DEFAULT_PERSPECTIVE_POINTS;

    matrix_t matrix;
    MatrixOperations::DeriveMatrix(source_points, target_points, matrix);
    std::vector<float> m(&matrix[0], &matrix[16]);

    return m;
}


std::vector<float> WarpConfigurator::GetInversePrescaleMatrix()
{
    const float output_ratio = static_cast<float>(_output_res.first) / static_cast<float>(_output_res.second);
    const float input_ratio = static_cast<float>(_input_res.first) / static_cast<float>(_input_res.second);

    float width = 1.0f;
    float height = 1.0f;

    if(input_ratio > output_ratio)
        height = height * (output_ratio/input_ratio);
    else
        width = width * (input_ratio/output_ratio);

    float x1 = (1.0f - width) / 2.0f;
    float y1 = (1.0f - height) / 2.0f;
    float x2 = x1 + width;
    float y2 = y1 + height;

    point_t source_points[4] = DEFAULT_PERSPECTIVE_POINTS;
    point_t target_points[4] = {{x1, y1}, {x2, y1}, {x1, y2}, {x2, y2}};

    matrix_t matrix;
    MatrixOperations::DeriveMatrix(source_points, target_points, matrix);
    std::vector<float> m(&matrix[0], &matrix[16]);

    return m;
}


void WarpConfigurator::SetPreRadial(float x, float y, float k1, float k2)
{
    _radial._focus[0] = get_norm_val(x, static_cast<float>(_output_res.first));
    _radial._focus[1] = get_norm_val(y, static_cast<float>(_output_res.second));
    _radial._k1 = k1;
    _radial._k2 = k2;
}


void WarpConfigurator::GetPreRadial(float& x, float& y, float& k1, float& k2) const
{
    x = get_denorm_val(_radial._focus[0], static_cast<float>(_output_res.first));
    y = get_denorm_val(_radial._focus[1], static_cast<float>(_output_res.second));
    k1 = _radial._k1;
    k2 = _radial._k2;
}


void WarpConfigurator::SetFisheyeLensFov(float v)
{
    _fe_lens_fov = degrees_to_radians(v);
}


float WarpConfigurator::GetFisheyeLensFov() const
{
    return radians_to_degrees(_fe_lens_fov);
}


void WarpConfigurator::SetEquiRectEnable(bool v)
{
    _fe_transform = v ? EFisheyeTransform::EquiRectangular : EFisheyeTransform::None;
}


bool WarpConfigurator::GetEquiRectEnable() const
{
    return _fe_transform == EFisheyeTransform::EquiRectangular;
}


void WarpConfigurator::SetFisheyeRadius(float v)
{
    _fe_r = v;
}


float WarpConfigurator::GetFisheyeRadius() const
{
    return _fe_r;
}


void WarpConfigurator::SetEquiRectFovH(float v)
{
    _equirect_fov_h = degrees_to_radians(v);
}


float WarpConfigurator::GetEquiRectFovH() const
{
    return radians_to_degrees(_equirect_fov_h);
}


void WarpConfigurator::SetEquiRectFovV(float v)
{
    _equirect_fov_v = degrees_to_radians(v);
}


float WarpConfigurator::GetEquiRectFovV() const
{
    return radians_to_degrees(_equirect_fov_v);
}


void WarpConfigurator::SetFishToPanoEnable(bool v)
{
    _fe_transform = v ? EFisheyeTransform::Panorama : EFisheyeTransform::None;
}

bool WarpConfigurator::GetFishToPanoEnable() const
{
    return _fe_transform == EFisheyeTransform::Panorama;
}

void WarpConfigurator::SetFishToPanoLatitudeMin(float v)
{
    _ftp_latitude_min = degrees_to_radians(v);
}

float WarpConfigurator::GetFishToPanoLatitudeMin() const
{
    return radians_to_degrees(_ftp_latitude_min);
}

void WarpConfigurator::SetFishToPanoLatitudeMax(float v)
{
    _ftp_latitude_max = degrees_to_radians(v);
}

float WarpConfigurator::GetFishToPanoLatitudeMax() const
{
    return radians_to_degrees(_ftp_latitude_max);
}

void WarpConfigurator::SetFishToPanoLongitude(float v)
{
    _ftp_longitude = degrees_to_radians(v);
}

float WarpConfigurator::GetFishToPanoLongitude() const
{
    return radians_to_degrees(_ftp_longitude);
}

void WarpConfigurator::SetFishToPanoFovH(float v)
{
    _ftp_fov_h= degrees_to_radians(v);
}

float WarpConfigurator::GetFishToPanoFovH() const
{
    return radians_to_degrees(_ftp_fov_h);
}


void WarpConfigurator::SetFtpInsideOut(bool v)
{
    _ftp_insideout = v;
}


bool WarpConfigurator::GetFtpInsideOut() const
{
    return _ftp_insideout;
}


bool WarpConfigurator::ValidateResolution(const std::pair<uint32_t, uint32_t>& res) const
{
    return (res.first > 0 && res.first <= MAX_WIDTH) && (res.second > 0 && res.second <= MAX_HEIGHT);
}


void WarpConfigurator::SetInputResolution(const uint32_t width, const uint32_t height)
{
    const auto res = std::make_pair(width, height);

    if(ValidateResolution(res))
        _input_res = res;
}


std::pair<uint32_t, uint32_t> WarpConfigurator::GetInputResolution() const
{
    return _input_res;
}


void WarpConfigurator::SetOutputResolution(const uint32_t width, const uint32_t height)
{
    const auto res = std::make_pair(width, height);

    if(ValidateResolution(res) && (_output_res != res))
    {
        _output_res = res;

        const float output_ratio = static_cast<float>(_output_res.first) / static_cast<float>(_output_res.second);
        float ratio = 1.0f / output_ratio;
        _kt.UpdateRatio(ratio);
        _at.UpdateRatio(ratio);
    }
}


std::pair<uint32_t, uint32_t> WarpConfigurator::GetOutputResolution() const
{
    return _output_res;
}


ITransformerPtr WarpConfigurator::GetPreRadialTransformer(bool inverse)
{
    ITransformerPtr p{nullptr};

    auto float_not_zero = [](const float v)->bool
    {
        return (fabs(v - 0.0f) > FLT_EPSILON);
    };

    if(float_not_zero(_radial._k1) || float_not_zero(_radial._k2))
    {
        const float output_ratio = static_cast<float>(_output_res.first) / static_cast<float>(_output_res.second);
        float ratio = 1.0f / output_ratio;

        if(inverse)
            p = std::make_shared<TransformerRadialInverse>(ratio, _radial._focus, _radial._k1, _radial._k2);
        else
            p = std::make_shared<TransformerRadial>(ratio, _radial._focus, _radial._k1, _radial._k2);
    }

    return p;
}


ITransformerPtrs WarpConfigurator::GetTransformersForward(bool use_corners, bool updateShrinkToFitZoom/* = true*/)
{
    if(updateShrinkToFitZoom && _shrink_to_fit)
    {
        float hZoom, vZoom;
        FindShrinkToFitZoom(use_corners, hZoom, vZoom);
        SetZoom(std::min(hZoom, vZoom));
    }

    ITransformerPtrs transformers;

    if(GetMaintainRatio())
        if(GetInputResolution() != GetOutputResolution())
            transformers.push_back(ITransformerPtr(new TransformerPerspective(GetPrescaleMatrix())));

    auto pre_radial_transformer = GetPreRadialTransformer(false);

    if (pre_radial_transformer)
        transformers.push_back(std::move(pre_radial_transformer));

    if(GetArcConf().IsConfigured())
        transformers.push_back(ITransformerPtr(new TransformerArc(GetArcConf(), false)));

    transformers.push_back(ITransformerPtr(new TransformerPerspective(GetPerspectiveMatrix(use_corners))));

    return transformers;
}


ITransformerPtrs WarpConfigurator::GetTransformersReverse(bool use_corners, bool updateShrinkToFitZoom/* = true*/)
{
    if(updateShrinkToFitZoom && _shrink_to_fit)
    {
        float hZoom, vZoom;
        FindShrinkToFitZoom(use_corners, hZoom, vZoom);
        SetZoom(std::min(hZoom, vZoom));
    }

    ITransformerPtrs transformers;

    transformers.push_back(ITransformerPtr(new TransformerPerspective(GetInversePerspectiveMatrix(use_corners))));

    if(GetArcConf().IsConfigured())
        transformers.push_back(ITransformerPtr(new TransformerArc(GetArcConf(), true)));

    auto pre_radial_transformer = GetPreRadialTransformer();

    if (pre_radial_transformer)
        transformers.push_back(std::move(pre_radial_transformer));

    if(GetMaintainRatio())
        if(GetInputResolution() != GetOutputResolution())
            transformers.push_back(ITransformerPtr(new TransformerPerspective(GetInversePrescaleMatrix())));

    return transformers;
}


uint32_t WarpConfigurator::ConvertToArbitraryMesh(bool fromCorners)
{
    ITransformerPtrs transformers = GetTransformersForward(fromCorners);

    _arbitrary_points.clear();
    _arbitrary_knots = 9;

    for (uint32_t yIndex = 0; yIndex < 9; yIndex++)
    {
        for (uint32_t xIndex = 0; xIndex < 9; xIndex++)
        {
            float y = (float)yIndex / 8.0f;
            float x = (float)xIndex / 8.0f;

            for (auto& transformer : transformers)
                transformer->Transform(x, y);

            _arbitrary_points.push_back({x, y});
        }
    }

    return _arbitrary_knots;
}


void WarpConfigurator::SetArbitraryKnotsNum(uint32_t arbitrary_knots, bool maintainShape)
{
    if (maintainShape)
    {
        if (arbitrary_knots == (2 * _arbitrary_knots) - 1)
        {
            // Add points inbetween
            auto arbitraryMesh = GetArbitraryMesh();
            _arbitrary_points.clear();

            const auto& meshPoints = arbitraryMesh->GetMeshPoints();
            uint32_t meshWidth = ((_arbitrary_knots + 1) * ArbitraryMesh::NUM_SEGMENTS) + 1;
            uint32_t halfSegments = ArbitraryMesh::NUM_SEGMENTS / 2;

            for (uint32_t y = 0; y < arbitrary_knots; y++)
            {
                uint32_t meshY = (y + 2) * halfSegments;

                for (uint32_t x = 0; x < arbitrary_knots; x++)
                {
                    uint32_t meshX = (x + 2) * halfSegments;
                    uint32_t meshIndex = (meshY * meshWidth) + meshX;
                    _arbitrary_points.push_back(meshPoints[meshIndex]);
                }
            }

            _arbitrary_knots = arbitrary_knots;
            return;
        }
        else if (_arbitrary_knots == (2 * arbitrary_knots) - 1)
        {
            // Remove points inbetween
            std::vector<ArbitraryMesh::TPoint> newPoints;
            for (uint32_t y = 0; y < _arbitrary_knots; y += 2)
            {
                for (uint32_t x = 0; x < _arbitrary_knots; x += 2)
                {
                    uint32_t pointIndex = (y * _arbitrary_knots) + x;
                    newPoints.push_back(_arbitrary_points[pointIndex]);
                }
            }

            _arbitrary_points = std::move(newPoints);
            _arbitrary_knots = arbitrary_knots;
            return;
        }
    }

    _arbitrary_points.clear();
    _arbitrary_knots = arbitrary_knots;
    float step = 1.0f/static_cast<float>(_arbitrary_knots - 1);
    for(uint32_t i=0; i<_arbitrary_knots; ++i)
    {
        float v = i * step;

        for(uint32_t j=0; j<_arbitrary_knots; ++j)
        {
            float h = j * step;
            _arbitrary_points.push_back({h,v});
        }
    }
}

// At the moment arbitrary mesh has the same number
// of vertical and horizontal control knots
uint32_t WarpConfigurator::GetArbitraryKnotsNum() const
{
    return _arbitrary_knots;
}


void WarpConfigurator::SetArbitraryKnot(const uint32_t idx, const float pixel_x, const float pixel_y)
{
    const float width = static_cast<float>(_output_res.first);
    const float height = static_cast<float>(_output_res.second);
    SetArbitraryKnotNorm(idx, get_norm_val(pixel_x, width), get_norm_val(pixel_y, height));
}


std::pair<float, float> WarpConfigurator::GetArbitraryKnot(const uint32_t idx) const
{
    const auto [xn, yn] = GetArbitraryKnotNorm(idx);
    const auto x = get_denorm_val(xn, static_cast<float>(_output_res.first));
    const auto y = get_denorm_val(yn, static_cast<float>(_output_res.second));
    return std::make_pair(x, y);
}


void WarpConfigurator::SetArbitraryKnotNorm(const uint32_t idx, const float x, const float y)
{
    try
    {
        auto& p = _arbitrary_points.at(idx);
        p.SetX(x);
        p.SetY(y);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Error setting arbitary knot: " << e.what() << std::endl;
    }
}


std::pair<float, float> WarpConfigurator::GetArbitraryKnotNorm(const uint32_t idx) const
{
    float x = 0.0f, y = 0.0f;

    if(idx < _arbitrary_points.size())
    {
        const auto& p = _arbitrary_points.at(idx);
        x = p.GetX();
        y = p.GetY();
    }
    else
        std::cerr << "Error getting arbitary knot: index out of range" << std::endl;

    return std::make_pair(x, y);
}


WarpMeshPtr WarpConfigurator::GenerateMeshFromAffine(bool use_corners)
{
    const auto& input_resolution =  GetInputResolution();
    const auto& output_resolution = GetOutputResolution();

    auto mesh = WarpMesh::Create(input_resolution, output_resolution, _hw_ctx);

    // Fill in mesh here
    if(mesh)
    {        
        ITransformerPtrs transformers = GetTransformersReverse(use_corners);

        auto mesh_task = [&](const uint32_t v_begin, const uint32_t v_end) {
            const uint32_t fract_bits = mesh->GetFractBits();

            for( uint32_t v = v_begin; v < v_end; ++v )
            {
                const uint32_t v_output = v * mesh->GetStep();
                // Normalise to 0..1 range
                const float y_norm = (float)v_output/(float)(output_resolution.second);

                mesh_node_t* node = mesh->GetRow(v);

                // Warp engine renders a whole number of blocks
                // some resolutions need their dimensions rounded accordingly
                for( uint32_t h = 0; h < mesh->GetHNodes(); ++h )
                {
                    const uint32_t h_output = h * mesh->GetStep();

                    // Normalise to 0..1 range
                    const float x_norm = (float)h_output/(float)(output_resolution.first);
                    
                    float y = y_norm;
                    float x = x_norm;

                    for (auto& transformer : transformers)
                        transformer->Transform(x, y);

                    // Denormalise using input resolution
                    // this will anamorphically fit input image in the output frame
                    const int32_t h_input = (int32_t)((float)(1 << fract_bits) * (x * (float)(input_resolution.first)) + std::copysignf(0.5f, x));
                    const int32_t v_input = (int32_t)((float)(1 << fract_bits) * (y * (float)(input_resolution.second)) + std::copysignf(0.5f, y));

                    node[h]._x = h_input;
                    node[h]._y = v_input;
                }
            }
        };

#ifndef ALT_SINGLE_THREADED
        const uint32_t hw_concurr = std::min(static_cast<uint32_t>(std::thread::hardware_concurrency()), (mesh->GetVNodes() >> 7));

        if(hw_concurr > 1)
        {
            std::vector<std::future<void>> f(hw_concurr - 1);
            const uint32_t thread_v_step = mesh->GetVNodes() / hw_concurr;
            uint32_t thread_v_begin = 0;

            for(uint32_t i = 0; i < (hw_concurr - 1); ++i, thread_v_begin += thread_v_step)
            {
                f[i] = std::async(std::launch::async, mesh_task, thread_v_begin, thread_v_begin + thread_v_step);
            }

            mesh_task(thread_v_begin, mesh->GetVNodes());

            for(uint32_t i = 0; i < (hw_concurr - 1); ++i) f[i].get();
        }
        else
#endif /* ALT_SINGLE_THREADED    */
            mesh_task(0, mesh->GetVNodes());
    }

    return mesh;
}


WarpMeshPtr WarpConfigurator::GenerateMeshFromFixed()
{
    bool use_corners = false;
    return GenerateMeshFromAffine(use_corners);
}


WarpMeshPtr WarpConfigurator::GenerateMeshFromCorners()
{
    bool use_corners = true;
    return GenerateMeshFromAffine(use_corners);
}


std::shared_ptr<ArbitraryMesh> WarpConfigurator::GetArbitraryMesh()
{
    auto spArbitraryMesh = std::make_shared<ArbitraryMesh>(GetArbitraryKnotsNum(), GetArbitraryKnotsNum(), _arbitrary_points);
    return spArbitraryMesh;
}


WarpMeshPtr WarpConfigurator::GenerateMeshFromArbitrary()
{
    WarpMeshPtr warpMesh{ nullptr };

    auto arbitraryMesh = GetArbitraryMesh();

    if (arbitraryMesh)
    {
        const auto& input_resolution = GetInputResolution();
        const auto& output_resolution = GetOutputResolution();

        warpMesh = WarpMesh::Create(input_resolution, output_resolution, _hw_ctx);

        if(warpMesh)
            arbitraryMesh->ToReverse(input_resolution.first, input_resolution.second, output_resolution.first, output_resolution.second, *warpMesh);
    }

    return warpMesh;
}


WarpMeshPtr WarpConfigurator::GenerateMeshFromFisheye()
{
    const auto& input_resolution =  GetInputResolution();
    const auto& output_resolution = GetOutputResolution();

    auto mesh = WarpMesh::Create(input_resolution, output_resolution, _hw_ctx);

    if(mesh)
    {
        if(GetFishToPanoEnable())
        {
            const float vertical_FOV = std::max(degrees_to_radians(10.0f), std::fabs(_ftp_latitude_max - _ftp_latitude_min));
            const float r_fisheye = (std::min(output_resolution.first, output_resolution.second) / 2) * _fe_r;
            const float x_offset = static_cast<float>(output_resolution.first) / 2.0f;
            const float y_offset = static_cast<float>(output_resolution.second) / 2.0f;

            const uint32_t fract_bits = mesh->GetFractBits();
            auto p_mesh_node = mesh->GetRow(0);

            for(std::size_t v = 0; v < mesh->GetVNodes(); ++v)
            {
                const int32_t y_pano = static_cast<int32_t>(v * mesh->GetStep());
                const float latitude = _ftp_latitude_min + ((float)(y_pano) / (float)(output_resolution.second)) * vertical_FOV;

                float r = 0.0f;

                if(GetFtpInsideOut())
                    r = r_fisheye - r_fisheye * sinf(latitude);
                else
                    r = r_fisheye * sinf(latitude);

                for(std::size_t h = 0; h < mesh->GetHNodes(); ++h)
                {
                    const int32_t x_pano = static_cast<int32_t>(h * mesh->GetStep());
                    const float longitude = _ftp_longitude + ((float)x_pano / (float)output_resolution.first) * _ftp_fov_h;
                    const float x = r * cosf(longitude);
                    const float y = r * sinf(longitude);

                    // Convert from Cartesian coordinates centered at (x_offset, y_offset)
                    // to image coordinates with an inverted y-axis.
                    // Cartesian y increases upward, while image y increases downward.
                    const float x_fish = (x + x_offset);
                    const float y_fish = (y_offset - y);

                    const int32_t x_int = static_cast<int32_t>(x_fish * static_cast<float>(1 << fract_bits) + 0.5f);
                    const int32_t y_int = static_cast<int32_t>(y_fish * static_cast<float>(1 << fract_bits) + 0.5f);
                    
                    p_mesh_node->_x = x_int;
                    p_mesh_node->_y = y_int;
                    ++p_mesh_node;
                }
            }
        }
        else
        {
            std::function<void(float&, float&)> transformer{};

            if(GetEquiRectEnable())
            {
                auto t = std::make_shared<TransformerEquirectangular>(point_t{0.5f, 0.5f}, _equirect_fov_h, _equirect_fov_v, _fe_lens_fov, _fe_r);

                transformer = [t = std::move(t)](float& x, float&y){t->Transform(x, y);};
            }
            else
                transformer = [](float& x, float&y){};

            const uint32_t fract_bits = mesh->GetFractBits();

            const uint32_t v_begin = 0;
            const uint32_t v_end = mesh->GetVNodes();

            for( uint32_t v = v_begin; v < v_end; ++v )
            {
                const uint32_t v_output = v * mesh->GetStep();
                // Normalise to 0..1 range
                const float y_norm = (float)v_output/(float)(output_resolution.second);

                mesh_node_t* node = mesh->GetRow(v);

                // Warp engine renders a whole number of blocks
                // some resolutions need their dimensions rounded accordingly
                for( uint32_t h = 0; h < mesh->GetHNodes(); ++h )
                {
                    const uint32_t h_output = h * mesh->GetStep();

                    // Normalise to 0..1 range
                    const float x_norm = (float)h_output/(float)(output_resolution.first);
                    
                    float y = y_norm;
                    float x = x_norm;

                    transformer(x, y);

                    // Denormalise using input resolution
                    // this will anamorphically fit input image in the output frame
                    const int32_t h_input = (int32_t)((float)(1 << fract_bits) * (x * (float)(input_resolution.first)) + std::copysignf(0.5f, x));
                    const int32_t v_input = (int32_t)((float)(1 << fract_bits) * (y * (float)(input_resolution.second)) + std::copysignf(0.5f, y));

                    node[h]._x = h_input;
                    node[h]._y = v_input;
                }
            }
        }
    }

    return mesh;
}


bounding_rect_t WarpConfigurator::GetBoundingRectangle(bool use_corners)
{
    // Hard-code number of nodes for now
    const uint32_t h_nodes = 16;
    const uint32_t v_nodes = 9;

    bounding_rect_t br = { 1.0f, 1.0f, 0.0f, 0.0f };
    ITransformerPtrs transformers = GetTransformersForward(use_corners, false);

    // Transform points
    for(uint32_t vi=0; vi<=v_nodes; ++vi)
    {
        for(uint32_t hi=0; hi<=h_nodes; ++hi)
        {
            float x = (float)hi/(float)h_nodes;
            float y = (float)vi/(float)v_nodes;

            for(auto p: transformers)
                p->Transform(x, y);

            if(x < br._x1) br._x1 = x;
            if(y < br._y1) br._y1 = y;
            if(x > br._x2) br._x2 = x;
            if(y > br._y2) br._y2 = y;
        }
    }

    return br;
}


void WarpConfigurator::FindShrinkToFitZoom(bool use_corners, float& hZoom, float& vZoom)
{
    SetZoom(1.0f);

    bounding_rect_t br = GetBoundingRectangle(use_corners);

    float minx = br._x1;
    float miny = br._y1;
    float maxx = br._x2;
    float maxy = br._y2;

    float midx = (minx + maxx)/2.0f;
    float fx1 = (maxx-midx)/(1.0f-midx);
    float fx2 = (midx-minx)/(midx);
    hZoom = 1.0f/std::max(fx1, fx2);

    float midy = (miny + maxy)/2.0f;
    float fy1 = (maxy-midy)/(1.0f-midy);
    float fy2 = (midy-miny)/(midy);
    vZoom = 1.0f/std::max(fy1, fy2);
}

} /* namespace intel_vvp_warp */

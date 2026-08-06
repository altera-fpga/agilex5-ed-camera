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
#include <utility>
#include "WarpConfiguratorTypes.h"
#include "WarpMesh.h"
#include "MatrixOperations.h"
#include "Transformer.h"
#include "ArbitraryMesh.h"


namespace intel_vvp_warp
{

class WarpConfigurator
{
public:
	WarpConfigurator(const WarpHwContextPtr& hw_ctx);
	virtual ~WarpConfigurator();

	void SetInputResolution(const uint32_t width, const uint32_t height);
	std::pair<uint32_t, uint32_t> GetInputResolution() const;
	void SetOutputResolution(const uint32_t width, const uint32_t height);
	std::pair<uint32_t, uint32_t> GetOutputResolution() const;

	void Reset();
    void ResetCorners();

	// To allow sub-pixel values
	// sizes, positions and offsets are floats
	void SetHSize(float pixels);
	float GetHSize() const;
	void SetVSize(float pixels);
	float GetVSize() const;
	void SetHOffset(float pixels);
	float GetHOffset() const;
	void SetVOffset(float pixels);
	float GetVOffset() const;

	void SetRotate(float angle);
	float GetRotate() const;

	void SetHMirror(bool v);
	bool GetHMirror() const;
	void SetVMirror(bool v);
	bool GetVMirror() const;

	void SetZoom(float zoom);
	float GetZoom() const;

	void SetHKeystone(float angle);
	float GetHKeystone() const;
	void SetVKeystone(float angle);
	float GetVKeystone() const;

	void SetPreRadial(float x, float y, float k1, float k2);
	void GetPreRadial(float& x, float& y, float& k1, float& k2) const;

	void SetFOV(float angle);
	float GetFOV() const;
	void SetVAxisOffset(float v);
	float GetVAxisOffset() const;

	void SetCorner(const ECornerId& corner, float pixel_x, float pixel_y);
	void SetCornerNorm(const ECornerId& corner, float x, float y);
    std::pair<float, float> GetCorner(const ECornerId& corner) const;
	std::pair<float, float> GetCornerNorm(const ECornerId& corner) const;

	void SetArc(const EEdgeId& edge, float val);
	float GetArc(const EEdgeId& edge) const;
	bool IsArcConfigured() const
	{
		return _arc_conf.IsConfigured();
	}
	const arc_conf_t& GetArcConf() const
	{
		return _arc_conf;
	}

	void SetMaintainRatio(bool val);
	bool GetMaintainRatio() const
	{
		return _maintain_ratio;
	}
	void SetShrinkToFit(bool shrink_to_fit)
	{
		_shrink_to_fit = shrink_to_fit;
	}
	bool GetShrinkToFit() const
	{
		return _shrink_to_fit;
	}

	void SetArbitraryKnotsNum(uint32_t num, bool maintainShape = false);
	uint32_t GetArbitraryKnotsNum() const;
	void SetArbitraryKnot(const uint32_t idx, const float pixel_x, const float pixel_y);
    std::pair<float, float> GetArbitraryKnot(const uint32_t idx) const;
	void SetArbitraryKnotNorm(const uint32_t idx, const float x, const float y);
    std::pair<float, float> GetArbitraryKnotNorm(const uint32_t idx) const;

    // Fisheye lens parameters
    void SetFisheyeLensFov(float v);
    float GetFisheyeLensFov() const;
    void SetFisheyeRadius(float v);
    float GetFisheyeRadius() const;

    // Equirectangular mapping
    void SetEquiRectEnable(bool v);
    bool GetEquiRectEnable() const;
    void SetEquiRectFovH(float v);
    float GetEquiRectFovH() const;
    void SetEquiRectFovV(float v);
    float GetEquiRectFovV() const;

    // Fisheye to panorama mapping
    void SetFishToPanoEnable(bool v);
    bool GetFishToPanoEnable() const;
    void SetFishToPanoLatitudeMin(float v);
    float GetFishToPanoLatitudeMin() const;
    void SetFishToPanoLatitudeMax(float v);
    float GetFishToPanoLatitudeMax() const;
    void SetFishToPanoLongitude(float v);
    float GetFishToPanoLongitude() const;
    void SetFishToPanoFovH(float v);
    float GetFishToPanoFovH() const;

	WarpMeshPtr GenerateMeshFromFixed();
	WarpMeshPtr GenerateMeshFromCorners();
	WarpMeshPtr GenerateMeshFromArbitrary();
	WarpMeshPtr GenerateMeshFromFisheye();

	uint32_t ConvertToArbitraryMesh(bool fromCorners);

    void SetFtpInsideOut(bool v);
    bool GetFtpInsideOut() const;

private:

	bool ValidateResolution(const std::pair<uint32_t, uint32_t>& res) const;

	void TransformPerspectivePoints(point_t* p, std::size_t n, bool use_corners);

	ITransformerPtr GetPreRadialTransformer(bool inverse = true);

	bounding_rect_t GetBoundingRectangle(bool use_corners);
	void FindShrinkToFitZoom(bool use_corners, float& hZoom, float& vZoom);

	std::vector<float> GetPerspectiveMatrix(bool use_corners);
	std::vector<float> GetInversePerspectiveMatrix(bool use_corners);

	std::vector<float> GetPrescaleMatrix();
	std::vector<float> GetInversePrescaleMatrix();

	ITransformerPtrs GetTransformersForward(bool use_corners, bool updateShrinkToFitZoom = true);
	ITransformerPtrs GetTransformersReverse(bool use_corners, bool updateShrinkToFitZoom = true);
	WarpMeshPtr GenerateMeshFromAffine(bool use_corners = false);

	std::shared_ptr<ArbitraryMesh> GetArbitraryMesh();

    std::pair<uint32_t, uint32_t> _input_res;
    std::pair<uint32_t, uint32_t> _output_res;

	TransformerKeystone _kt;
	TransformerAffine _at;

	radial_t _radial;

	bool _shrink_to_fit;

	point_t _corners[ETotalCorners];

	arc_conf_t _arc_conf;

	bool _maintain_ratio;

	// Currently arbitrary mesh has the same number
	// of vertical and horizontal control knots
	uint32_t _arbitrary_knots;
	std::vector<ArbitraryMesh::TPoint> _arbitrary_points;

	WarpHwContextPtr _hw_ctx;

    // Fisheye lens transforms
    enum class EFisheyeTransform
    {
        EquiRectangular,
        Panorama,
        None
    };

    EFisheyeTransform _fe_transform;

    // Fisheye lens params
    float _fe_lens_fov;
    float _fe_r;

    // Fisheye to panorama mapping
	float _ftp_latitude_min;
	float _ftp_latitude_max;
	float _ftp_longitude;
	float _ftp_fov_h;
    bool _ftp_insideout;

    // Equirectangular mapping
    float _equirect_fov_h;
    float _equirect_fov_v;
};

} /* namespace intel_vvp_warp */

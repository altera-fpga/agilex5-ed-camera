/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "IDialogHandler.h"

#include <vector>
#include <memory>
#include <functional>
#include "ICommand.h"

class PiPLayer
{
public:
    uint32_t _index;
    std::string _name;
    int32_t _x;
    int32_t _y;
    uint32_t _width;
    uint32_t _height;
    int32_t _minimumX;
    int32_t _minimumY;
    int32_t _maximumX;
    int32_t _maximumY;
    uint32_t _minimumWidth;
    uint32_t _minimumHeight;
    uint32_t _maximumWidth;
    uint32_t _maximumHeight;
    uint32_t _zIndex;
    uint8_t _alpha;
    std::string _colour;
    bool _fixedSize;
    bool _fixedAspectRatio;
};

class PiPDetails
{
public:
    PiPDetails(uint32_t outputWidth, uint32_t outputHeight);
	uint32_t AddLayer(std::string name, int32_t x, int32_t y,
                      uint32_t width, uint32_t height,
                      int32_t minimumX, int32_t minimumY,
                      int32_t maximumX, int32_t maximumY,
					  uint32_t minimumWidth, uint32_t minimumHeight,
					  uint32_t maximumWidth, uint32_t maximumHeight,
					  int32_t zIndex = -1,
					  uint8_t alpha = 255,
                      std::string colour = "#00aeef",
					  bool fixedSize = false,
                      bool fixedAspectRatio = false);

    uint32_t _outputWidth;
    uint32_t _outputHeight;
	std::vector<PiPLayer> _layers;
    PiPLayer& GetLayer(int32_t index);
};

using PiPLayerUpdateCB = std::function<void(uint32_t clientId, PiPLayer& pipLayer)>;

class PictureInPictureHandler : public IDialogHandler
{
public:
	PictureInPictureHandler(PiPLayerUpdateCB layerUpdateCB, std::shared_ptr<PiPDetails> spPiPDetails);
    ~PictureInPictureHandler();

    // IDialogHandler
    void ShowDialog(uint32_t clientID) override;

    std::shared_ptr<PiPDetails> GetDetails() { return _spPiPDetails; }

private:
    void PictureInPictureLayerUpdateCB(ParamListPtr& parameter_list);

    PiPLayerUpdateCB _layerUpdateCB;
    std::shared_ptr<PiPDetails> _spPiPDetails;
};

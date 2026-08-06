/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "PictureInPictureHandler.h"
#include "CommonApplicationBase.h"

static const char* pipLayerUpdateCommand = "PictureInPictureLayerUpdate";

PictureInPictureHandler::PictureInPictureHandler(PiPLayerUpdateCB layerUpdateCB,
												 std::shared_ptr<PiPDetails> spPiPDetails)
:	_layerUpdateCB(std::move(layerUpdateCB))
,	_spPiPDetails(std::move(spPiPDetails))
{
    ICommandMap* pCommandMap = CommonApplicationBase::Get()->GetCommandMap();

    // Register a command to use as callback for the update picture dialog
    pCommandMap->Add(new UiServerCommand(pipLayerUpdateCommand, [this](ParamListPtr& spParameterList) { PictureInPictureLayerUpdateCB(spParameterList); }));
}

PictureInPictureHandler::~PictureInPictureHandler()
{
    ICommandMap* pCommandMap = CommonApplicationBase::Get()->GetCommandMap();
	std::string strCommand(pipLayerUpdateCommand);
	pCommandMap->Remove(strCommand);
}

void PictureInPictureHandler::ShowDialog(uint32_t clientID)
{
	PiPDialogUiUpdate uiUpdate(clientID, _spPiPDetails);
	uiUpdate.Notify();
}

void PictureInPictureHandler::PictureInPictureLayerUpdateCB(ParamListPtr& spParameterList)
{
	if (spParameterList->CheckNumParams(7))
	{
		uint32_t layerIndex = 0;
		uint32_t p = 0;
		AtUtils::FromString(spParameterList->at(p++), layerIndex);

		if (layerIndex < _spPiPDetails->_layers.size())
		{
			PiPLayer& layer = _spPiPDetails->_layers[layerIndex];
			AtUtils::FromString(spParameterList->at(p++), layer._x);
			AtUtils::FromString(spParameterList->at(p++), layer._y);
			AtUtils::FromString(spParameterList->at(p++), layer._zIndex);
			AtUtils::FromString(spParameterList->at(p++), layer._width);
			AtUtils::FromString(spParameterList->at(p++), layer._height);
			uint32_t alpha = 0;
			AtUtils::FromString(spParameterList->at(p++), alpha);
			layer._alpha = (uint8_t)alpha;

			if (_layerUpdateCB)
				_layerUpdateCB(spParameterList->GetClientID(), layer);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

PiPDetails::PiPDetails(uint32_t outputWidth, uint32_t outputHeight)
:	_outputWidth(outputWidth)
,	_outputHeight(outputHeight)
{
}

uint32_t PiPDetails::AddLayer(std::string name, int32_t x, int32_t y,
							  uint32_t width, uint32_t height,
							  int32_t minimumX, int32_t minimumY,
							  int32_t maximumX, int32_t maximumY,
							  uint32_t minimumWidth, uint32_t minimumHeight,
							  uint32_t maximumWidth, uint32_t maximumHeight,
							  int32_t zIndex,
							  uint8_t alpha,
							  std::string colour,
							  bool fixedSize,
							  bool fixedAspectRatio)
{
	uint32_t nextIndex = (uint32_t)_layers.size();
	if (zIndex < 0)
		zIndex = (int32_t)nextIndex;
	PiPLayer newLayer{nextIndex, std::move(name),
					  x, y, width, height,
					  minimumX, minimumY,
					  maximumX, maximumY,
					  minimumWidth, minimumHeight,
					  maximumWidth, maximumHeight,
					  (uint32_t)zIndex,
					  alpha,
					  std::move(colour),
					  fixedSize,
					  fixedAspectRatio};
	_layers.push_back(std::move(newLayer));
	return nextIndex;
}

PiPLayer& PiPDetails::GetLayer(int32_t index)
{
	if (static_cast<size_t>(index) < _layers.size())
		return _layers[index];
	else
	{
		static PiPLayer pipLayer{};
		return pipLayer;
	}
}

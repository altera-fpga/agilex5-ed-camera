/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "Encoder_Interface.h"
#ifdef GSTREAMER_PIPELINE
    //#include "gstreamer_pipeline/Gstreamer_Encoder.h"
#endif
#ifdef DSHOW_PIPELINE
    #include "dshow_pipeline/DShow_Encoder.h"
#endif

#include "Tiff_Encoder.h"

Encoder_Interface::Encoder_Interface()
{
}

Encoder_Interface::~Encoder_Interface()
{
}

Encoder_Interface *Encoder_Interface::create_encoder(const std::string &encoder_name)
{
    if ((encoder_name == "tif") || (encoder_name == "tiff"))
    {
        return new Tiff_Encoder;
    }
    return NULL;
}

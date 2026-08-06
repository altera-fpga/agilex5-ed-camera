/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __DPTX_APP_DEFS_H__
#define __DPTX_APP_DEFS_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

//-- EDID DEFS
//-- EDID split in to 128byte blocks
#define EDID_BLOCK_SIZE                    128
#define EDID_INDEX_EXT_BLOCK_COUNT         126

//-- Basic Display Parameters
#define EDID_INDEX_BDP_START               20
#define EDID_INDEX_BDP_DIGITAL_INPUT       0x80
#define EDID_INDEX_BDP_BIT_DEPTH_MASK      0x70
#define EDID_INDEX_BDP_BIT_DEPTH_10        0x30

#define EDID_INDEX_DESCRIPTOR1_START       54
#define EDID_INDEX_DESCRIPTOR4_START       108
#define EDID_INDEX_DESCRIPTOR_SIZE         18

//-- Monitor Descriptor
#define EDID_INDEX_MDB_NAME_TEXT_START     5
#define EDID_INDEX_MDB_NAME_TEXT_LENGTH    (EDID_INDEX_DESCRIPTOR_SIZE-EDID_INDEX_MDB_NAME_TEXT_START)

//-- CEA extensions
#define EDID_INDEX_EXT_TAG                 0
#define EDID_INDEX_EXT_REV                 1
#define EDID_INDEX_EXT_DTD_START           2
#define EDID_INDEX_EXT_CEA_SUPPORT         3
#define EDID_INDEX_EXT_DB_START            4

//-- HDMI 2.0 Data Block definitions
#define EDID_VIDEO_DATA_BLOCK              2
#define EDID_VENDOR_SPECIFIC_DATA_BLOCK    3

//-- Minimum length of the VSDB that we parse
#define EDID_INDEX_EXT_VSDB_MIN_LENGTH     5
#define EDID_INDEX_EXT_HFVSDB_MIN_LENGTH   7

//-- VIC codes of interest
#define EDID_720P30_VIC                    62
#define EDID_720P60_VIC                    4
#define EDID_1080P30_VIC                   34
#define EDID_1080P60_VIC                   16
#define EDID_2160P30_VIC                   95
#define EDID_2160P60_VIC                   97
#define EDID_NA_VIC                        0xff

#define EDID_DTD_SIZE                      18

#define MAX_SUPPORTED_FORMATS              28
#define FORMATS_REG_MASK             ((0x1<<MAX_SUPPORTED_FORMATS)-1)
#define FORMATS_REG_10BPC            0x80000000

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DPTX_APP_DEFS_H__ */
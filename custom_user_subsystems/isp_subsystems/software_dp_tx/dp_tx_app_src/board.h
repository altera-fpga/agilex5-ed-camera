/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __BOARD_H__
#define __BOARD_H__


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "alt_types.h"

void board_configure();
void board_tx_freq(int freq);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOARD_H__ */

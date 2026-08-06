/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __INTEL_PIO_REGS_H__
#define __INTEL_PIO_REGS_H__

#define ALTERA_AVALON_PIO_DATA          (0)
#define ALTERA_AVALON_PIO_DIRECTION     (1)
#define ALTERA_AVALON_PIO_IRQ_MASK      (2)
#define ALTERA_AVALON_PIO_EDGE_CAP      (3)
#define ALTERA_AVALON_PIO_SET_BIT       (4)
#define ALTERA_AVALON_PIO_CLEAR_BITS    (5)
 
/* Defintions for direction-register operation with bi-directional PIOs */
#define ALTERA_AVALON_PIO_DIRECTION_INPUT  0
#define ALTERA_AVALON_PIO_DIRECTION_OUTPUT 1

#endif /* __INTEL_PIO_REGS_H__ */

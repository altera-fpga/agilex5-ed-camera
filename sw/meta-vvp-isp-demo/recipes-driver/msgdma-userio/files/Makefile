# *******************************************************************************
# Copyright (C) Altera Corporation
#
# This code and the related documents are Altera copyrighted materials and your
# use of them is governed by the express license under which they were provided to
# you ("License"). This code and the related documents are provided as is, with no
# express or implied warranties other than those that are expressly stated in the 
# License.
# *******************************************************************************/
obj-m := msgdma_userio.o
msgdma_userio-y += msgdma_userio_platform.o  msgdma_userio_chr.o 

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean
	
modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

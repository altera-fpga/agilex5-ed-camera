/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "intel_coredla.h"
#include <fcntl.h>

#include <sys/mman.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

static struct _memmap  memmap_instance = {-1, NULL, 0};

int intel_coredla_init(intel_coredla_instance* instance, intel_vvp_core_base base)
{
    int init_ret;

    if (instance == NULL) return kIntelVvpCoreInstanceErr;
    
    instance->_pDdrBase = (uint8_t *)0x60000000U;
    instance->core_instance.base = base;

    if(memmap_instance._fd == -1)
    {
        memmap_instance._sz = 0x20000000; // 512MB should be enough for everything, right?

        memmap_instance._fd = open("/dev/mem", O_RDWR);

        if (memmap_instance._fd != -1)
        {
            memmap_instance._cpuPointer = mmap(0, memmap_instance._sz, PROT_READ | PROT_WRITE, MAP_SHARED, memmap_instance._fd, (uint64_t)instance->_pDdrBase);
            if (memmap_instance._cpuPointer == MAP_FAILED)
            {
                printf("Memory map failed!\n");
            }
        }
        else
        {
            printf("Failed to open /dev/mem!\n");
        }
    }
    
    return kIntelVvpCoreOk;
}
 
void intel_coredla_RegisterISR(intel_coredla_instance* instance, interrupt_service_routine_signature func, void *data)
{
}

void intel_coredla_csr_write(intel_coredla_instance* instance, uint32_t addr, uint32_t data)
{
    INTEL_COREDLA_REG_IOWR(instance, addr, data);
}

uint32_t intel_coredla_csr_read(intel_coredla_instance* instance, uint32_t addr)
{
    return INTEL_COREDLA_REG_IORD(instance, addr);
}

// Copies the block of data from the host to the FPGA
// memcpy is not used as this can cause multiple transfers of the AXI bus depending
// on the implementation of memcpy
void intel_coredla_ddr_write(intel_coredla_instance* instance, uint64_t addr, uint64_t length, const void *data)
{
    // Transfer the remaining 32bits of data
    /*
    volatile uint32_t *pDeviceMem32 = (volatile uint32_t*)((uint8_t*)(instance->_cpuPointer) + addr);
    const uint32_t *host_addr32 = (const uint32_t*)(data);
    while( length >= sizeof(uint32_t) ) {
        *pDeviceMem32++ = *host_addr32++;
        length -= sizeof(uint32_t);
    }
    */
    /*
    printf("WRITE: %p <- Size: %lx\n", instance->_pDdrBase + addr, length);
    printf("Debug prints\n");
    for (int i = 0; i < 32; i++)
    {
        printf("%p: %x\n", instance->_pDdrBase + addr + (i * 4), ((uint32_t*)data)[i]);
    }
    */
    memcpy((uint8_t*)(memmap_instance._cpuPointer) + addr, data, length);
    msync((uint8_t*)(memmap_instance._cpuPointer) + addr, length, MS_SYNC);
}

// Copies the block of data from the FPGA to the host
// memcpy is not used as this can cause multiple transfers of the AXI bus depending
// on the implementation of memcpy
void intel_coredla_ddr_read(intel_coredla_instance* instance, uint64_t addr, uint64_t length, void *data)
{
    // Transfer the data in 32bit chunks
    /*
    volatile const uint32_t *pDeviceMem32 = (volatile const uint32_t*)((uint8_t*)(instance->_cpuPointer) + addr);
    uint32_t *host_addr32 = (uint32_t *)(data);
    while (length >= sizeof(uint32_t)) {
        *host_addr32++ = *pDeviceMem32++;
        length -= sizeof(uint32_t);
    }
    */
    //printf("READ: %p -> Size: %lx\n", instance->_pDdrBase + addr, length);
    msync((uint8_t*)(memmap_instance._cpuPointer) + addr, length, MS_SYNC);
    memcpy(data, (uint8_t*)(memmap_instance._cpuPointer) + addr, length);
}

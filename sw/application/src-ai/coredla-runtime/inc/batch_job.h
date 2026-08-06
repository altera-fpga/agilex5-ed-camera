/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef BATCH_JOB_H
#define BATCH_JOB_H

#include <stdint.h>

class BatchJob {
 public:
  // @param inputArray - ptr to CPU array containing input data to be copied to DDR
  // @param async - if true will start emulator inferences in new threads
  // blocking function
  virtual void LoadInputFeatureToDDR(
    void* inputArray
  #ifdef EMULATOR_DLA_PLUGIN
    , bool async=false
  #endif
  ) = 0;
  // @param outputArray - ptr to CPU array where the output data in DDR is copied into
  // outputArray must be allocated by the caller (size >= output_size_ddr)
  // blocking function
  virtual void ReadOutputFeatureFromDDR(void* outputArray) const = 0;
  virtual void ScheduleInputFeature() const = 0;
  // @param async - if true will start emulator inferences in new threads
  virtual void StartDla(
  #ifdef EMULATOR_DLA_PLUGIN
    bool async=false
  #endif
  ) = 0;
  virtual ~BatchJob() {}
  virtual int GetThreadId() const { return 0; }
  virtual uint32_t GetInputAddrDDR() const = 0;
  virtual uint32_t GetOutputAddrDDR() const = 0;

};

#endif

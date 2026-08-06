/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "compiled_result.h"          //dla::CompiledResult
#include "device.h"                   //Device
#include "device_memory_allocator.h"  //DeviceMemoryAllocator
#include "graph_job.h"                //GraphJob
#include "mmd_wrapper.h"              //MmdWrapper

#include <condition_variable>  //std::condition_variable
#include <cstdint>             //uint64_t
#include <map>                 //std::map
#include <memory>              //std::unique_ptr
#include <mutex>               //std::mutex
#include <vector>              //std::vector

// OpenVINO plugin developer api
#include "openvino/runtime/iplugin.hpp"

class StreamControllerComms;

// The interface of the interrupt service routine dictates that all the data the ISR needs must be passed in through
// one pointer of type void *. Package it up here. WaitForDla() uses jobsWaited and jobsFinished to determine if a job
// has already finished or it still needs wait. The ISR only updates jobsFinished, so jobsWaited is only a member of
// CoreDlaDevice. The mutex and condition variable are used to synchronize between InterruptServiceRoutine() and
// WaitForDla(). All of these are replicated per CoreDLA IP instance, hence the use of vector.
// base_multiplier and prevCount are used to handle the jobsFinished wrap-around that could happen in the hardware CSR
// as the CSR is only 32-bit wide but the jobsFinished is 64-bit wide
struct InterruptServiceRoutineData {
  MmdWrapper* mmdWrapper;
  std::vector<uint64_t> jobsFinished;
  std::vector<uint32_t> base_multiplier;
  std::vector<uint32_t> prevCount;
  std::vector<uint32_t> desc_queue_diag;
  std::vector<std::mutex> isrMutex;
  std::vector<std::condition_variable> isrCondVar;
};

/*! DlaDevice class represents a DLA device mapped using the MMD + OPAE SW stack
 * On construction, dynamically loads MMD library at runtime and initialized the state of MMD
 * Implememts functions that wrap various MMD calls to read/write to DDR/CSR and process HW interrupts
 */
class CoreDlaDevice : public Device {
 public:
  GraphJob* CreateGraphJob(const dla::CompiledResult* compiledResult,
#ifndef USE_OLD_COREDLA_DEVICE
                           size_t numPipelines,
#else
                           uint64_t numPipelines,
#endif
                           int instance,
                           // This param is unused for HW runtime! So why inlcude it? CoreDLA utilizes base pointers
                           // for both HW and SW emulator runtime. The software emulator has output file where as currently the
                           // HW runtime does not.
                           const std::string export_dir,
                           const std::string on_chip_parameters_export_dir);
  // Return number of DLA jobs completed till now
  // Used for debugging
  int GetNumInferencesCompleted(int instance) const override { return isrData_.jobsFinished.at(instance); }
  // Must be called when there are no active jobs on DLA
  // Returns the total time taken by DLA jobs on hardware (in milliseconds)
  double GetActiveHWTimeMs(int instance) const override;
  // Must be called when there are no active jobs on DLA
  // Returns the average of time taken per job (in milliseconds)
  // Avg Time per job < Active Time
  double GetAvgHWTimePerJobMs(uint64_t num_jobs, int instance) const override;
  // Must be called when there are no active jobs on DLA
  // Returns the number of memory read made by the input feature reader
  uint64_t GetNumInputFeatureMemoryReads(int instance) const override;
  // Must be called when there are no active jobs on DLA
  // Returns the number of memory read made by the filter reader
  uint64_t GetNumFilterMemoryReads(int instance) const override;
  // Must be called when there are no active jobs on DLA
  // Returns the number of memory writes made by the output feature writer
  uint64_t GetNumOutputFeatureMemoryWrites(int instance) const override;

 private:
  // Read one 32-bit value from the debug network, return value indicates whether read was successful. A read can fail
  // if the module number and address have not been implemented. The debug network is fault tolerant to both read
  // requests never being accepted as well as read responses never being produced.
  bool ReadDebugCsr(uint32_t moduleNum, uint32_t address, int instance, uint32_t& readData, bool verbose = false) const;

#ifndef USE_OLD_COREDLA_DEVICE
  // Must be called when there are no active jobs on DLA
  // Returns total number of clocks by DLA jobs on hardware.
  uint64_t GetClocksActive(int instance) const;

  // Must be called when there are no active jobs on DLA
  // Returns the clocks of all jobs
  uint64_t GetClocksAllJobs(int instance) const;
#endif

  uint64_t GetNumInputFeatureMemoryReadsTotal(int instance) const;

  uint64_t GetNumFilterMemoryReadsTotal(int instance) const;

  uint64_t GetNumOutputFeatureMemoryWritesTotal(int instance) const;

 public:
  // Modules attached to the debug network have a ROM to specify the offset and description of the registers. Traverse
  // this ROM, then return a map of key/value pairs, where the key is a human readable string describing what kind of
  // information the debug register contains, and the value is the data of the debug register.
  DebugNetworkData ReadDebugNetwork(int instance) const override;

  CoreDlaDevice(uint32_t waitForDlaTimeoutSeconds, bool enableLogging = false, const std::string jtagPath = "");
  virtual ~CoreDlaDevice();
  int GetSizeCsrDescriptorQueue() const override;
  double GetCoreDlaClockFreq() const override;
  int GetNumInstances() const override { return numInstances_; }
  void WaitForDla(int instance, size_t threadId = 0, std::function<bool()> isCancelled = nullptr) override;  // threadId is optional and for debugging purpose only
  std::string SchedulerGetStatus() const override;
  bool InitializeScheduler(uint32_t sourceBufferSize, uint32_t dropSourceBuffers, uint32_t numInferenceRequests,
                           const std::string source_fifo_file="") override;
  // Query board information through MMD.
  float GetBoardTemperature() const override;
  float GetBoardInputPower() const override;
  bool HasStreamController() const override;
  void OnlineReconfig(const std::string mifs_path) const override;
  ov::SoPtr<ov::IRemoteContext> GetIRemoteContext();

 private:
  std::unique_ptr<DeviceMemoryAllocator[]> ddrAllocator_;
  std::vector<std::unique_ptr<GraphJob>> allGraphJobs_;
  int numInstances_;
  MmdWrapper mmdWrapper_;
  InterruptServiceRoutineData isrData_;
  std::vector<uint64_t> jobsWaited_;
#ifndef USE_OLD_COREDLA_DEVICE
  std::vector<uint64_t> startClocksActive;
  std::vector<uint64_t> startClockAllJobs;
#endif
  std::vector<uint64_t> startNumInputFeatureMemoryReads;
  std::vector<uint64_t> startNumFilterMemoryReads;
  std::vector<uint64_t> startNumOutputFeatureMemoryWrites;
  std::shared_ptr<StreamControllerComms> spStreamControllerComms_;
  bool runtimePolling_;
  uint32_t waitForDlaTimeoutSeconds_;
  std::string streaming_input_pipe_;
  std::string jtagPath_;
};

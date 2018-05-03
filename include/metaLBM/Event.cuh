#pragma once

#include "Commons.h"
#include "Event.h"
#include "Options.h"

namespace lbm {

template <>
class Event<Architecture::GPU> : public Event<Architecture::Generic> {
 private:
  cudaEvent_t event;

 public:
  Event() { CUDA_CALL(cudaEventCreate(&event)); }

  ~Event() { CUDA_CALL(cudaEventDestroy(event)); }

  void record(Stream<Architecture::GPU>& stream) {
    CUDA_CALL(cudaEventRecord(event, stream.get()));
  }

  void wait(Stream<Architecture::GPU>& stream) {
    CUDA_CALL(cudaEventWaitEvent(stream.get(), event, 0));
  }
};

}  // namespace lbm

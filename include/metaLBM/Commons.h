#pragma once

#include <iostream>

#ifdef __NVCC__
#include <cuda.h>
#include <cuda_runtime_api.h>

#define LBM_CUDA_CALL(call)                                                \
  {                                                                        \
    cudaError_t error = call;                                              \
    if (error != cudaSuccess) {                                            \
      std::cerr << "Cuda failure " << __FILE__ << ":" << __LINE__ << ": '" \
                << cudaGetErrorString(error) << "'" << std::endl;          \
      exit(0);                                                             \
    }                                                                      \
  }

#define LBM_HOST __host__
#define LBM_DEVICE __device__
#define LBM_CONSTANT __constant__
#define LBM_GLOBAL __global__
#define LBM_INLINE __forceinline__

#ifdef USE_NVTX
#include <nvToolsExt.h>

class Tracer {
  static constexpr int numberColors = 7;
  const uint32_t colors[numberColors] = {0x0000ff00, 0x000000ff, 0x00ffff00,
                                         0x00ff00ff, 0x0000ffff, 0x00ff0000,
                                         0x00ffffff};

 public:
  // HOST DEVICE
  Tracer(const char* name, int colorID) {
    nvtxEventAttributes_t eventAttribute = {0};
    eventAttribute.version = NVTX_VERSION;
    eventAttribute.size = NVTX_EVENT_ATTRIB_STRUCT_SIZE;
    eventAttribute.colorType = NVTX_COLOR_ARGB;
    eventAttribute.color = colors[colorID % numberColors];
    eventAttribute.messageType = NVTX_MESSAGE_TYPE_ASCII;
    eventAttribute.message.ascii = name;
    nvtxRangePushEx(&eventAttribute);
  }

  // HOST DEVICE
  ~Tracer() { nvtxRangePop(); }
};

#define LBM_SCOREP_INSTRUMENT_ON(name, colorID) \
  Tracer uniq_name_using_macros(name, colorID);

#else
#define LBM_SCOREP_INSTRUMENT_ON(name, colorID)
#endif  // USE_NVTX

#else  // __NVCC__
#define LBM_CUDA_CALL(call)

#define LBM_HOST
#define LBM_DEVICE
#define LBM_CONSTANT
#define LBM_GLOBAL
#define LBM_INLINE __always_inline

#ifdef USE_SCOREP
#include <scorep/SCOREP_User.h>

#define INSTRUMENT_ON(name, colorID) \
  { SCOREP_USER_REGION(name, SCOREP_USER_REGION_TYPE_FUNCTION) }

#else  // USE_SCOREP
#define LBM_SCOREP_INSTRUMENT_ON(name, colorID)
#endif  // USE_SCOREP
#endif  // __NVCC__

#define LBM_SCOREP_INSTRUMENT_OFF(name, colorID)

namespace lbm {}

#pragma once

#include <stdlib.h>
#include <cstring>

#include "Commons.h"
#include "Options.h"

#ifdef USE_FFTW
#include <fftw3-mpi.h>
#define MALLOC_CPU fftw_malloc
#define FREE_CPU fftw_free
#else
#define MALLOC_CPU malloc
#define FREE_CPU free
#endif

namespace lbm {

template <class U, Architecture architecture>
class DynamicArray {};

template <class U>
class DynamicArray<U, Architecture::Generic> {
 protected:
  unsigned int numberElements;
  U* dArrayPtr;

  DynamicArray(const unsigned int numberElements_in = 0, U* dArrayPtr_in = NULL)
      : numberElements(numberElements_in), dArrayPtr(dArrayPtr_in) {}

 public:
  LBM_DEVICE LBM_HOST U& operator[](int i) { return dArrayPtr[i]; }

  LBM_DEVICE LBM_HOST const U& operator[](int i) const { return dArrayPtr[i]; }

  LBM_DEVICE LBM_HOST U* data(const unsigned offset = 0) {
    return dArrayPtr + offset;
  }

  LBM_DEVICE LBM_HOST const U* data(const unsigned offset = 0) const {
    return dArrayPtr + offset;
  }

  LBM_DEVICE LBM_HOST unsigned int size() { return numberElements; }

  LBM_DEVICE LBM_HOST unsigned int size() const { return numberElements; }
};

template <class U>
class DynamicArray<U, Architecture::CPU>
    : public DynamicArray<U, Architecture::Generic> {
 private:
  using Base = DynamicArray<U, Architecture::Generic>;

 protected:
  using Base::dArrayPtr;
  using Base::numberElements;

 public:
  using Base::operator[];
  using Base::data;
  using Base::size;

  DynamicArray() : Base() {}

  DynamicArray(const unsigned int numberElements_in)
    : Base(numberElements_in)
  {
    dArrayPtr = (U*)MALLOC_CPU(numberElements_in * sizeof(U));
  }

  DynamicArray(const DynamicArray<U, Architecture::CPU>& dArray_in)
    : Base(dArray_in.size())
  {
    dArrayPtr = (U*)MALLOC_CPU(dArray_in.size() * sizeof(U));
    copyFrom(dArray_in);
  }

  ~DynamicArray() {
    if (dArrayPtr) {
      FREE_CPU(dArrayPtr);
      dArrayPtr = NULL;
    }
  }

  void copyFrom(const DynamicArray<U, Architecture::CPU>& other) {
    memcpy(dArrayPtr, other.data(), other.size() * sizeof(U));
  }

  void copyTo(DynamicArray<U, Architecture::CPU>& other) const {
    memcpy(other.data(), dArrayPtr, numberElements * sizeof(U));
  }
};

template <class U, Architecture architecture>
bool operator==(DynamicArray<U, architecture> const& lhs,
                DynamicArray<U, architecture> const& rhs) {
  if (lhs.size() == rhs.size) {
    for (auto i = 0; i < lhs.size(); ++i) {
      if (!(lhs[i] == rhs[i])) {
        return false;
      }
    }
    return true;
  } else {
    return false;
  }
}

}  // namespace lbm

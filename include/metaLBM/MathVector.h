#pragma once

#include <cmath>
#include <fstream>
#include <sstream>

#include "StaticArray.h"

namespace lbm {

template <class U, unsigned int NumberComponents>
class MathVector {
 public:
  StaticArray<U, NumberComponents> sArray;

  LBM_HOST LBM_DEVICE U& operator[](int i) { return sArray[i]; }

  LBM_HOST LBM_DEVICE const U& operator[](int i) const { return sArray[i]; }

  LBM_HOST LBM_DEVICE U* data() { return sArray.data(); }

  LBM_HOST LBM_DEVICE const U* data() const { return sArray.data(); }

  LBM_HOST LBM_DEVICE MathVector<U, NumberComponents>& operator=(
      const MathVector<U, NumberComponents> other) {
    sArray = other.sArray;
    return *this;
  }

  LBM_HOST LBM_DEVICE MathVector<U, NumberComponents>& operator=(
      const U other[NumberComponents]) {
    sArray = other;
    return *this;
  }

  LBM_HOST LBM_DEVICE inline U sum() {
    U sumR = 0;
    for (auto iC = 0; iC < NumberComponents; ++iC) {
      sumR += sArray[iC];
    }

    return sumR;
  }

  LBM_HOST LBM_DEVICE inline U dot(
      const MathVector<U, NumberComponents>& other) {
    U dotR = sArray[0] * other[0];
    for (auto iC = 1; iC < NumberComponents; ++iC) {
      dotR += sArray[iC] * other[iC];
    }

    return dotR;
  }

  LBM_HOST LBM_DEVICE inline U norm2() const {
    U norm2R = sArray[0] * sArray[0];
    for (auto iC = 1; iC < NumberComponents; ++iC) {
      norm2R += sArray[iC] * sArray[iC];
    }

    return norm2R;
  }

  LBM_HOST
  inline U norm() const { return sqrt(norm2()); }

  LBM_HOST LBM_DEVICE inline U volume() {
    U volumeR = sArray[0];
    for (auto iC = 1; iC < NumberComponents; ++iC) {
      volumeR *= sArray[iC];
    }

    return volumeR;
  }
};

template <class U, unsigned int NumberComponents>
LBM_HOST std::ostream& operator<<(std::ostream& os,
                                  const MathVector<U, NumberComponents>& mV) {
  os << mV.sArray;
  return os;
}

template <class U, unsigned int NumberComponents>
LBM_HOST std::ofstream& operator<<(std::ofstream& file,
                                   const MathVector<U, NumberComponents>& mV) {
  file << "\t\t\t\t";

  UnrolledFor<0, NumberComponents - 1>::Do(
      [&] LBM_HOST(int i) { file << mV[i] << " "; });

  file << mV[NumberComponents - 1];

  return file;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE bool operator==(
    MathVector<U, NumberComponents> const& lhs,
    MathVector<U, NumberComponents> const& rhs) {
  return lhs.sArray == rhs.sArray;
}

template <class U>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, 1>& operator+=(
    MathVector<U, 1>& lhs,
    const MathVector<U, 1>& rhs) {
  lhs[0] += rhs[0];
  return lhs;
}

template <class U>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, 2>& operator+=(
    MathVector<U, 2>& lhs,
    const MathVector<U, 2>& rhs) {
  lhs[0] += rhs[0];
  lhs[1] += rhs[1];
  return lhs;
}

template <class U>
LBM_HOST LBM_DEVICE MathVector<U, 3>& operator+=(MathVector<U, 3>& lhs,
                                                 const MathVector<U, 3>& rhs) {
  lhs[0] += rhs[0];
  lhs[1] += rhs[1];
  lhs[2] += rhs[2];
  return lhs;
}

template <class U>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, 1> operator+(
    const MathVector<U, 1>& mV_a,
    const MathVector<U, 1>& mV_b) {
  return MathVector<U, 1>{mV_a[0] + mV_b[0]};
}

template <class U>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, 2> operator+(
    const MathVector<U, 2>& mV_a,
    const MathVector<U, 2>& mV_b) {
  return MathVector<U, 2>{mV_a[0] + mV_b[0], mV_a[1] + mV_b[1]};
}

template <class U>
LBM_HOST LBM_DEVICE LBM_INLINE constexpr MathVector<U, 3> operator+(
    const MathVector<U, 3>& mV_a,
    const MathVector<U, 3>& mV_b) {
  return MathVector<U, 3>{mV_a[0] + mV_b[0], mV_a[1] + mV_b[1],
                          mV_a[2] + mV_b[2]};
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, NumberComponents>& operator*=(
    MathVector<U, NumberComponents>& mV,
    const U factor) {
#pragma unroll
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV[iC] *= factor;
  }

  return mV;
}

template <class U, class V, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, NumberComponents> operator*(
    const MathVector<U, NumberComponents>& mV,
    const V factor) {
  MathVector<U, NumberComponents> mV_result;
#pragma unroll
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV[iC] * (U)factor;
  }

  return mV_result;
}

template <class U, class V, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<U, NumberComponents> operator*(
    const V factor,
    const MathVector<U, NumberComponents>& mV) {
  MathVector<U, NumberComponents> mV_result;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV[iC] * (U)factor;
  }

  return mV_result;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<U, NumberComponents>& operator/=(
    MathVector<U, NumberComponents>& mV,
    const U factor) {
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV[iC] /= factor;
  }

  return mV;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<U, NumberComponents> operator/(
    const MathVector<U, NumberComponents>& mV,
    const U factor) {
  MathVector<U, NumberComponents> mV_result;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV[iC] / factor;
  }

  return mV_result;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<U, NumberComponents>& operator-=(
    MathVector<U, NumberComponents>& lhs,
    const MathVector<U, NumberComponents>& rhs) {
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    lhs[iC] -= rhs[iC];
  }

  return lhs;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<U, NumberComponents> operator-(
    const MathVector<U, NumberComponents>& mV_a,
    const MathVector<U, NumberComponents>& mV_b) {
  MathVector<U, NumberComponents> mV_result;
#pragma unroll
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV_a[iC] - mV_b[iC];
  }

  return mV_result;
}

template <unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<unsigned int, NumberComponents>
operator-(const MathVector<unsigned int, NumberComponents>& mV_a,
          const MathVector<unsigned int, NumberComponents>& mV_b) {
  MathVector<unsigned int, NumberComponents> mV_result;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV_a[iC] - mV_b[iC];
  }

  return mV_result;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<unsigned int, NumberComponents>
operator-(const MathVector<unsigned int, NumberComponents>& mV_a,
          const MathVector<U, NumberComponents>& mV_b) {
  MathVector<unsigned int, NumberComponents> mV_result;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV_a[iC] - (unsigned int)mV_b[iC];
  }

  return mV_result;
}

template <class U, unsigned int NumberComponents>
LBM_HOST LBM_DEVICE LBM_INLINE MathVector<unsigned int, NumberComponents>
operator-(const MathVector<U, NumberComponents>& mV_a,
          const MathVector<unsigned int, NumberComponents>& mV_b) {
  MathVector<unsigned int, NumberComponents> mV_result;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = (unsigned int)mV_a[iC] - mV_b[iC];
  }

  return mV_result;
}

LBM_HOST LBM_DEVICE LBM_INLINE MathVector<unsigned int, 3> operator-(
    const MathVector<unsigned int, 3>& mV_a,
    const MathVector<unsigned int, 3>& mV_b) {
  return {mV_a[0] - mV_b[0], mV_a[1] - mV_b[1], mV_a[2] - mV_b[2]};
}

template <unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<unsigned int, 3> operator-(
    const MathVector<unsigned int, 3>& mV_a,
    const MathVector<unsigned int, NumberComponents>& mV_b) {
  MathVector<unsigned int, 3> mV_result = mV_a;
  for (auto iC = 0; iC < NumberComponents; ++iC) {
    mV_result[iC] = mV_a[iC] - mV_b[iC];
  }

  return mV_result;
}

template <unsigned int NumberComponents>
LBM_HOST LBM_DEVICE MathVector<unsigned int, 3> operator-(
    const MathVector<unsigned int, NumberComponents>& mV_a,
    const MathVector<unsigned int, 3>& mV_b) {
  MathVector<unsigned int, 3> mV_result = -1 * mV_b;
  for (auto iC = 0; iC < 3; ++iC) {
    mV_result[iC] = mV_a[iC] - mV_b[iC];
  }

  return mV_result;
}

template <class T, class U, unsigned int Dimension>
struct Project {
  LBM_HOST LBM_DEVICE static inline MathVector<T, Dimension> Do(
      const MathVector<U, 3>& mV) {
    MathVector<T, Dimension> mVProjected{{(T)0}};

    for (auto iD = 0; iD < Dimension; ++iD) {
      mVProjected[iD] = mV[iD];
    }

    return mVProjected;
  }
};

template <class T, unsigned int Dimension>
struct ProjectAndLeave1 {
  LBM_HOST LBM_DEVICE static inline MathVector<T, 3> Do(
      const MathVector<T, 3>& mV) {
    MathVector<T, 3> mVProjected = {(T)1, (T)1, (T)1};

    for (auto iD = 0; iD < Dimension; ++iD) {
      mVProjected[iD] = mV[iD];
    }

    return mVProjected;
  }
};

template <class T, unsigned int Dimension>
struct ProjectPadRealAndLeave1 {
  LBM_HOST LBM_DEVICE LBM_INLINE static MathVector<T, 3> Do(
      const MathVector<T, 3>& mV) {
    MathVector<T, 3> mVProjected = {(T)1, (T)1, (T)1};
#pragma unroll
    for (auto iD = 0; iD < Dimension; ++iD) {
      mVProjected[iD] = mV[iD];
    }

    mVProjected[Dimension - 1] = 2 * (mV[Dimension - 1] / 2 + (T)1);

    return mVProjected;
  }
};

template <class T, unsigned int Dimension>
struct ProjectAndLeave0 {
  LBM_HOST LBM_DEVICE static inline MathVector<T, 3> Do(
      const MathVector<T, 3>& mV) {
    MathVector<T, 3> mVProjected = {(T)0, (T)0, (T)0};

    for (auto iD = 0; iD < Dimension; ++iD) {
      mVProjected[iD] = mV[iD];
    }

    return mVProjected;
  }
};

template <class T, unsigned int Dimension>
struct ProjectPadComplexAndLeave1 {
  LBM_HOST LBM_DEVICE LBM_INLINE static MathVector<T, 3> Do(
      const MathVector<T, 3>& mV) {
    MathVector<T, 3> mVProjected = {(T)1, (T)1, (T)1};
#pragma unroll
    for (auto iD = 0; iD < Dimension - 1; ++iD) {
      mVProjected[iD] = mV[iD];
    }
    mVProjected[Dimension - 1] = mV[Dimension - 1] / ((T)2) + (T)1;

    return mVProjected;
  }
};

template <class T, class U, unsigned int NumberComponents>
struct Cast {
  LBM_HOST LBM_DEVICE static inline MathVector<U, NumberComponents> Do(
      const MathVector<T, NumberComponents>& mV) {
    MathVector<U, NumberComponents> mVCasted = {{(U)0}};

    for (auto iC = 0; iC < NumberComponents; ++iC) {
      mVCasted[iC] = (U)mV[iC];
    }

    return mVCasted;
  }
};

typedef MathVector<unsigned int, 3> Position;
typedef MathVector<int, 3> WaveNumber;

}  // namespace lbm

#ifndef OPTIONS_H
#define OPTIONS_H

#include "Commons.h"

namespace lbm {
  enum d {X, Y, Z};

  enum class LatticeType {Generic, D1Q3, D2Q5, D2Q9, D2Q37, D3Q15, D3Q19, D3Q27};

  enum class MemoryLayout {Generic, Default, SoA, AoS};
  enum class Implementation {Generic, MPI, NVSHMEM_OUT, NVSHMEM_IN};
  enum class PartitionningType {Generic, OneD, TwoD, ThreeD};
  enum class Architecture {Generic, CPU, GPU, CPUPinned};

  enum class DomainType {Generic, GlobalSpace, LocalSpace, HaloSpace, BufferXSpace,
      GlobalFourier, LocalFourier};

  enum class AlgorithmType {Generic, Pull};

  enum class InitDensityType{Homogeneous, Peak};
  enum class InitVelocityType{Homogeneous, Perturbated, Wave, Decay};

  enum class FieldType {Generic, Density, Velocity, Force, Alpha, Entropy};

  enum class EquilibriumType {Generic, Incompressible};

  enum class CollisionType {GenericSRT, BGK, ELBM, Approached_ELBM, ForcedNR_ELBM,
      ForcedBNR_ELBM, GenericMRT};

  enum class ForcingSchemeType {Generic, None, Guo, ShanChen, ExactDifferenceMethod};
  enum class ForceType {None, Generic, GenericTimeIndependent, GenericTimeDependent,
      Constant, Sinusoidal, Kolmogorov, ConstantShell, Time};

  enum class BoundaryType {Generic, None, Periodic, BounceBack_Halfway, Entropic};

  enum class AnalyzerType {Generic, FFT, TotalEnergy,
      TotalEnstrophy, EnergySpectrum};

  enum class InputOutput {Generic, None, DAT, VTR, HDF5, XDMF};
  enum class InputOutputDataFormat {Generic, ascii, binary};
  enum class InputOutputType {Generic, Serial, Parallel};
}

#endif // OPTIONS_H

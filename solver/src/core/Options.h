#ifndef OPTIONS_H
#define OPTIONS_H

namespace lbm {
  enum d {X, Y, Z};

  enum class LatticeType {Generic, D1Q3, D2Q5, D2Q9, D3Q15, D3Q19, D3Q27};

  enum class MemoryLayout {Generic, SoA, AoS};

  enum class PartitionningType {Generic, OneD, TwoD, ThreeD};

  enum class DomainType {Generic, Local, Halo, Global};

  // enum class AlgorithmType {Generic, FusedPull, Pull, FusedPush, Push};

  enum class InitDensityType{Homogeneous, Peak};
  enum class InitVelocityType{Homogeneous, Perturbated, Wave, Decay};

  enum class FieldType {Generic, Density, Velocity, Force, Alpha, Entropy};

  enum class EquilibriumType {Generic, Incompressible};

  enum class CollisionType {GenericSRT, BGK, ELBM, Approached_ELBM, ForcedNR_ELBM,
      ForcedBNR_ELBM, GenericMRT};

  enum class ForcingSchemeType {Generic, Guo, ShanChen, ExactDifferenceMethod};
  enum class ForceType {Generic, Constant, Sinusoidal, Kolmogorov,
      Turbulent, TurbulentTime, WavenumbersTime};

  enum class BoundaryType {Generic, Periodic, BounceBack_Halfway, Entropic};

  enum class ReaderType {Generic, VTR, HDF5};
  enum class WriterType {Generic, VTR, HDF5};

}

#endif // OPTIONS_H

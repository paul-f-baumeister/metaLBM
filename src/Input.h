#ifndef INPUT_H
#define INPUT_H

#include <string>

#include "metaLBM/Commons.h"
#include "metaLBM/Options.h"
#include "metaLBM/MathVector.h"

namespace lbm {
  using dataT = double;
  using Vector = MathVector<dataT, 3>;

  constexpr int numProcs = NPROCS;
  constexpr int numThreads = NTHREADS;

  constexpr LatticeType latticeT = LatticeType::D3Q27;
  constexpr int globalLengthX = GLOBAL_LENGTH_X;
  constexpr int globalLengthY = GLOBAL_LENGTH_Y;
  constexpr int globalLengthZ = GLOBAL_LENGTH_Z;

  constexpr unsigned int startIteration = 0;
  constexpr unsigned int endIteration = 100;
  constexpr unsigned int writeStep = 1;
  constexpr unsigned int backUpStep = 100;

  constexpr unsigned int scalarAnalysisStep = 1;
  constexpr unsigned int spectralAnalysisStep = 1;

  constexpr unsigned int successiveWriteStep = 2;

  constexpr AlgorithmType algorithmT = AlgorithmType::Pull;
  constexpr PartitionningType partitionningT = PartitionningType::OneD;
  constexpr Implementation implementationT = Implementation::MPI;
  constexpr MemoryLayout memoryL = MemoryLayout::SoA;

  constexpr dataT relaxationTime = 0.50000001;
  constexpr CollisionType collisionT = CollisionType::ForcedBNR_ELBM;
  constexpr EquilibriumType equilibriumT = EquilibriumType::Incompressible;

  constexpr InitDensityType initDensityT = InitDensityType::Homogeneous;
  constexpr dataT initDensityValue = 1.0;
  constexpr InitVelocityType initVelocityT = InitVelocityType::Homogeneous;
  constexpr Vector initVelocityVector = { {0.0, 0.0, 0.0} };

  constexpr ForcingSchemeType forcingSchemeT = ForcingSchemeType::ExactDifferenceMethod;
  constexpr ForceType forceT = ForceType::ConstantShell;

  constexpr Vector forceAmplitude = { {1e1, 1e1, 1e1} };
  constexpr Vector forceWaveLength = { {32.0, 32.0, 32.0} };
  constexpr int forcekMin = 1;
  constexpr int forcekMax = 2;

  constexpr BoundaryType boundaryT = BoundaryType::Generic;

  constexpr InputOutputFormat inputOutputFormatT = InputOutputFormat::ascii;
  constexpr auto prefix = "test_elbm";
  constexpr bool writeFieldInit = 1;
  constexpr bool writeAnalysisInit = 1;

  constexpr bool writeForce = 1;
  constexpr bool writeEntropy = 0;
  constexpr bool writeAlpha = 1;
  constexpr bool writeVorticity = 1;

  constexpr bool analyzeTotalEnergy = 1;
  constexpr bool analyzeTotalEnstrophy = 1;

  constexpr bool analyzeEnergySpectra = 1;
  constexpr bool analyzeEnstrophySpectra = 1;
}

#endif // INPUT_H

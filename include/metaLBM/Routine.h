#ifndef ROUTINE_H
#define ROUTINE_H

#include <chrono>

#include "Commons.h"
#include "Options.h"
#include "MathVector.h"
#include "FieldList.h"
#include "Distribution.h"
#include "AnalysisList.h"
#include "Algorithm.h"
#include "Writer.h"
#include "Communication.h"
#include "Computation.h"

namespace lbm {

  template<class T, Architecture architecture, Implementation implementation>
  class Routine {
  protected:
    Communication_ communication;

    FieldWriter_ fieldWriter;
    ScalarAnalysisWriter_ scalarAnalysisWriter;
    SpectralAnalysisWriter_ spectralAnalysisWriter;

    double initialMass;
    double finalMass;
    double differenceMass;
    double computationTime;
    double communicationTime;
    double writeTime;
    double totalTime;

    FieldList<T, architecture> fieldList;
    Distribution<T, DomainType::LocalSpace, architecture> distribution;

    Curl<double, Architecture::CPU, PartitionningType::OneD,
         L::dimD, L::dimD> curlVelocity;
    ScalarAnalysisList<T, architecture> scalarAnalysisList;
    SpectralAnalysisList<T, architecture> spectralAnalysisList;

    Algorithm<dataT, algorithmT, DomainType::LocalSpace, architecture,
              implementation> algorithm;
    Computation<Architecture::CPU, L::dimD> computationLocal;

  public:
    Routine(const MathVector<int, 3>& rankMPI_in,
            const MathVector<int, 3>& sizeMPI_in,
            const std::string& processorName_in,
            const unsigned int numberElements_in)
      : communication(rankMPI_in, sizeMPI_in, processorName_in)
      , fieldWriter(prefix, rankMPI_in, sizeMPI_in)
      , scalarAnalysisWriter(prefix, rankMPI_in, sizeMPI_in)
      , spectralAnalysisWriter(prefix, rankMPI_in, sizeMPI_in)
      , initialMass(0.0)
      , finalMass(0.0)
      , differenceMass(0.0)
      , computationTime(0.0)
      , communicationTime(0.0)
      , writeTime(0.0)
      , totalTime(0.0)
      , fieldList(rankMPI_in, numberElements_in, fieldWriter)
      , curlVelocity(fieldList.velocity.getMultiData(), fieldList.vorticity.getMultiData(),
                     Cast<unsigned int,ptrdiff_t, 3>::Do(gSD::sLength()).data(),
                     gFD::offset(rankMPI_in))
      , distribution("distribution",
                     initLocalDistribution<T, architecture>(fieldList.density,
                                                            fieldList.velocity))
      , scalarAnalysisList(fieldList, scalarAnalysisWriter, communication)
      , spectralAnalysisList(fieldList, spectralAnalysisWriter, communication)
      , algorithm(fieldList, distribution, communication)
      , computationLocal(lSD::sStart(), lSD::sEnd())
      {}


    void compute() {
      { INSTRUMENT_ON("Routine<T>::compute",1) }
      initializeLocalDistribution();

      writeFields(startIteration);
      writeAnalyses(startIteration);
      printInputs();

      initialMass = communication.reduce(fieldList.density.getLocalData());

      for(int iteration = startIteration+1; iteration <= endIteration; ++iteration) {
        algorithm.isStored = fieldWriter.getIsWritten(iteration)
        || scalarAnalysisWriter.getIsAnalyzed(iteration)
        || spectralAnalysisWriter.getIsAnalyzed(iteration);

        algorithm.iterate(iteration);

        writeFields(iteration);
        writeAnalyses(iteration);

        communicationTime += algorithm.getCommunicationTime();
        computationTime += algorithm.getComputationTime();
        totalTime += algorithm.getTotalTime();
      }

      finalMass = communication.reduce(fieldList.density.getLocalData());

      differenceMass = fabs(initialMass-finalMass)/initialMass;
      printOutputs();
    }


  protected:
    void printInputs() {

      communication.printInputs();

      if (communication.getRankMPI() == MathVector<int, 3>({0, 0, 0})) {
        std::cout.precision(15);
        std::cout << "-------------------OPTIONS-------------------" << std::endl
                  << "Lattice         : D" << L::dimD << "Q" << L::dimQ << std::endl
                  << "Global lengths  : " << gSD::sLength() << std::endl
                  << "Global memory   : " << gSD::sVolume()*sizeof(dataT) << "B" << std::endl
                  << "----------------------------------------------" << std::endl
                  << "NPROCS          : " << NPROCS << "" << std::endl
                  << "-------------------PARAMETERS-----------------" << std::endl
                  << "Relaxation time : " << relaxationTime << std::endl
                  << "Viscosity       : " << L::cs2 * (relaxationTime - 0.5) << std::endl
                  << "Start iteration : " << startIteration << std::endl
                  << "End iteration   : " << endIteration << std::endl
                  << "----------------------------------------------" << std::endl;
      }
    }

    void printOutputs() {
      if (communication.getRankMPI() == MathVector<int, 3>({0, 0, 0})) {
        std::cout << "--------------------OUTPUTS-------------------" << std::endl
                  << "Total time      : " << totalTime << " s" << std::endl
                  << "Comp time       : " << computationTime << " s" << std::endl
                  << "Comm time       : " << communicationTime << " s" << std::endl;

        const double mlups = (gSD::sVolume() * 1e-6)/(totalTime / (endIteration-startIteration+1));

        std::cout << "MLUPS           : " << mlups << std::endl
                  << "Initial mass    : " << initialMass << std::endl
                  << "Final mass      : " << finalMass << std::endl
                  << "% mass diff.    : " << differenceMass << std::endl
                  << "----------------------------------------------" << std::endl;
      }
    }

    void initializeLocalDistribution() {
      { INSTRUMENT_ON("Routine<T>::initializeLocalDistribution",2) }
      algorithm.unpack();
    }

    void writeFields(const unsigned int iteration) {
      { INSTRUMENT_ON("Routine<T>::writeFields",2) }

      if(fieldWriter.getIsWritten(iteration)) {
        fieldWriter.openFile(iteration);

        if(writeVorticity) curlVelocity.executeSpace();

        fieldList.writeFields();

      if(fieldWriter.getIsBackedUp(iteration)) {
        algorithm.pack();
        fieldWriter.writeField(distribution);
      }

      fieldWriter.closeFile();
      }
    }

    void writeAnalyses(const unsigned int iteration) {
      { INSTRUMENT_ON("Routine<T>::writeFields",2) }

      if(scalarAnalysisWriter.getIsAnalyzed(iteration)) {
        scalarAnalysisWriter.openFile(iteration);

        scalarAnalysisList.writeAnalyses(iteration);
        scalarAnalysisWriter.closeFile();
      }

      if(spectralAnalysisWriter.getIsAnalyzed(iteration)) {
        spectralAnalysisWriter.openFile(iteration);

        spectralAnalysisList.writeAnalyses(iteration);
        spectralAnalysisWriter.closeFile();
      }
    }


  };

} // namespace lbm

#endif // ROUTINE_H

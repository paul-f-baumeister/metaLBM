#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <chrono>
#include <utility>
#include <cstdio>

#include "Commons.h"
#include "Options.h"
#include "Domain.h"
#include "Lattice.h"
#include "Field.h"
#include "Distribution.h"
#include "Moment.h"
#include "Collision.h"
#include "Boundary.h"
#include "Communication.h"
#include "Computation.h"


namespace lbm {

  template<class T, AlgorithmType algorithmType, DomainType initDomainType,
    Architecture architecture, Implementation implementation>
  class Algorithm {
  public:
    HOST DEVICE
      void operator()(const MathVector<unsigned int, 3> iP);
  };


  template<class T, DomainType initDomainType,
           Architecture architecture, Implementation implementation>
    class Algorithm<T, AlgorithmType::Generic, initDomainType,
    architecture, implementation> {
  protected:
    T * localDensity_Ptr;
    T * * localVelocity_Ptr;
    T * * localForce_Ptr;
    T * localAlpha_Ptr;

    T * * localDistribution_Ptr;
    T * haloDistributionPrevious_Ptr;
    T * haloDistributionNext_Ptr;

    Packer<T> packer;
    Unpacker<T> unpacker;
    Communication<T, latticeT, algorithmT, memoryL,
                  partitionningT, implementation, L::dimD> communication;
    Collision_ collision;
    Moment<T> moment;

    Computation<architecture, L::dimD> computationLocal;
    Computation<architecture, L::dimD> computationHalo;

    std::chrono::duration<double> dtComputation;
    std::chrono::duration<double> dtCommunication;
    std::chrono::duration<double> dtTotal;

    bool isWritten;

    Algorithm(Field<T, 1, initDomainType, architecture, writeDensity>& densityField_in,
              Field<T, L::dimD, initDomainType, architecture, writeVelocity>& velocityField_in,
              Field<T, L::dimD, initDomainType, architecture, writeForce>& forceField_in,
              Field<T, 1, initDomainType, architecture, writeAlpha>& alphaField_in,
              Distribution<T, initDomainType, architecture>& distribution_in,
              Communication<T, latticeT, algorithmT, memoryL,
              partitionningT, implementation, L::dimD>& communication_in)
      : localDensity_Ptr(densityField_in.getLocalData())
      , localVelocity_Ptr(velocityField_in.getMultiData())
      , localForce_Ptr(forceField_in.getMultiData())
      , localAlpha_Ptr(alphaField_in.getLocalData())
      , localDistribution_Ptr(distribution_in.getMultiData())
      , haloDistributionPrevious_Ptr(distribution_in.getHaloDataPrevious())
      , haloDistributionNext_Ptr(distribution_in.getHaloDataNext())
      , communication(communication_in)
      , collision(relaxationTime, forceAmplitude, forceWaveLength, forcekMin, forcekMax)
      , moment()
      , computationLocal(lSD::sStart()+L::halo(),
                         lSD::sEnd()+L::halo())
      , computationHalo(hSD::start(),
                        hSD::end())
      , dtComputation()
      , dtCommunication()
      , dtTotal()
      , isWritten()
    {}

    DEVICE HOST
    void storeLocalFields(const MathVector<unsigned int, 3>& iP) {
      INSTRUMENT_OFF("Algorithm<T, AlgorithmType::Pull>::storeLocalFields",4)

      const unsigned int indexLocal = hSD::getIndexLocal(iP);

      localDensity_Ptr[indexLocal] = moment.getDensity();
      localAlpha_Ptr[indexLocal] = collision.getAlpha();

      for(unsigned int iD = 0; iD < L::dimD; ++iD) {
        localVelocity_Ptr[iD][indexLocal] = collision.getHydrodynamicVelocity()[iD];
        localForce_Ptr[iD][indexLocal] = collision.getForce()[iD];
      }

    }

  public:
    HOST
    void pack() {
      computationLocal.Do(packer, localDistribution_Ptr,
                          haloDistributionNext_Ptr);
    }

    HOST
    void unpack() {
      computationLocal.Do(unpacker, haloDistributionNext_Ptr,
                          localDistribution_Ptr);
    }

    double getCommunicationTime() {
      return dtCommunication.count();
    }

    double getComputationTime() {
      return dtComputation.count();
    }

    double getTotalTime() {
      return dtTotal.count();
    }
  };


  template<class T, DomainType initDomainType,
           Architecture architecture, Implementation implementation>
    class Algorithm<T, AlgorithmType::Pull, initDomainType,
                    architecture, implementation>
    : public Algorithm<T, AlgorithmType::Generic, initDomainType,
                       architecture, implementation> {
  private:
    using Base = Algorithm<T, AlgorithmType::Generic, initDomainType,
                           architecture, implementation>;

    using Base::localDensity_Ptr;
    using Base::localVelocity_Ptr;
    using Base::localForce_Ptr;
    using Base::localAlpha_Ptr;

    using Base::localDistribution_Ptr;
    using Base::haloDistributionPrevious_Ptr;
    using Base::haloDistributionNext_Ptr;

    using Base::communication;
    using Base::collision;
    using Base::moment;
    using Base::computationLocal;
    using Base::computationHalo;

    using Base::dtComputation;
    using Base::dtCommunication;
    using Base::dtTotal;
    using Base::isWritten;

    using Base::storeLocalFields;

    Boundary<T, BoundaryType::Periodic, AlgorithmType::Pull,
             partitionningT, implementation, L::dimD> periodicBoundary;

  public:
    Algorithm(Field<T, 1, initDomainType, architecture, writeDensity>& densityField_in,
              Field<T, L::dimD, initDomainType, architecture, writeVelocity>& velocityField_in,
              Field<T, L::dimD, initDomainType, architecture, writeForce>& forceField_in,
              Field<T, 1, initDomainType, architecture, writeAlpha>& alphaField_in,
              Distribution<T, initDomainType, architecture>& distribution_in,
              Communication<T, latticeT, algorithmT, memoryL,
              partitionningT, implementation, L::dimD>& communication_in)
      : Base(densityField_in, velocityField_in,
             forceField_in, alphaField_in,
             distribution_in,
             communication_in)
    {}

    HOST DEVICE
    void operator()(const MathVector<unsigned int, 3>& iP) {
      moment.calculateMoments(haloDistributionPrevious_Ptr, iP);

      collision.setForce(localForce_Ptr, iP, gSD::sOffset(communication.getRankMPI()));
      collision.setVariables(haloDistributionPrevious_Ptr, iP,
                             moment.getDensity(), moment.getVelocity());
      for(unsigned int iQ = 0; iQ < L::dimQ; ++iQ) {
        haloDistributionNext_Ptr[hSD::getIndex(iP, iQ)] =
          collision.calculate(haloDistributionPrevious_Ptr,
                              iP-uiL::celerity()[iQ], iQ);
      }

      if(isWritten) {
        storeLocalFields(iP);
      }
    }

    void setIsWritten(bool isWritten_in) {
      isWritten = isWritten_in;
    }

    HOST
    void iterate(const unsigned int iteration) {
      INSTRUMENT_ON("Algorithm<T, AlgorithmType::Pull>::iterate",2)

      std::swap(haloDistributionPrevious_Ptr, haloDistributionNext_Ptr);

      collision.update(iteration);

      auto t0 = std::chrono::high_resolution_clock::now();

      communication.communicateHalos(haloDistributionPrevious_Ptr);

      // TODO: Run only at the boundaries
      computationHalo.Do(periodicBoundary,
                         haloDistributionPrevious_Ptr);

      //boundary.apply(f_Previous.haloData());

      auto t1 = std::chrono::high_resolution_clock::now();

      computationLocal.Do(*this);

      auto t2 = std::chrono::high_resolution_clock::now();

      dtCommunication = (t1 - t0);
      dtComputation = (t2 - t1);
      dtTotal = (t2 - t0);
    }

    using Base::pack;
    using Base::unpack;
    using Base::getCommunicationTime;
    using Base::getComputationTime;
    using Base::getTotalTime;
  };


}

#endif // ALGORITHM_H

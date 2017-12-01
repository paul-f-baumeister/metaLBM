#ifndef COLLISION_H
#define COLLISION_H


#include "Commons.h"
#include "Options.h"
#include "Domain.h"
#include "Lattice.h"
#include "MathVector.h"
#include "Helpers.h"
#include "Equilibrium.h"
#include "Force.h"
#include "ForcingScheme.h"
#include "EntropicStep.h"

namespace lbm {

  template <class T, CollisionType collisionType>
  class Collision {};

  template <class T>
  class Collision<T, CollisionType::GenericSRT> {
  protected:
    T tau;

    Force_ force;
    ForcingScheme_ forcingScheme;
    Equilibrium_ equilibrium;

    Collision(const T tau_in,
              const MathVector<T, 3>& amplitude_in,
              const MathVector<T, 3>& waveLength_in)
      : tau(tau_in)
      , force(amplitude_in,
              waveLength_in)
      , forcingScheme(tau_in)
      , equilibrium()
    {}

  public:
    #pragma omp declare simd
    DEVICE HOST
    inline void setForce(const MathVector<unsigned int, 3>& iP_global) {
      force.setForce(iP_global);
    }

    #pragma omp declare simd
    DEVICE HOST
    inline const MathVector<T, L::dimD>& getForce() {
      return force.getForce();
    }

    #pragma omp declare simd
    DEVICE HOST
    inline const MathVector<T, L::dimD> getHydrodynamicVelocity() {
      return forcingScheme.calculateHydrodynamicVelocity(getForce());
    }

    #pragma omp declare simd
    DEVICE HOST
    inline T calculate(const T * RESTRICT f,
                              const MathVector<unsigned int, 3>& iP,
                              const unsigned int iQ) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::GenericSRT>::calculate")

      return ( (T) 1.0 - (T) 1.0 / tau) * f[hD::getIndex(iP, iQ)]
        + forcingScheme.calculateCollisionSource(force.getForce(), iQ)
        + (T) 1.0 / tau * equilibrium.calculate(iQ);
    }

    #pragma omp declare simd
    DEVICE HOST
    inline void setVariables(const T * RESTRICT f,
                             const MathVector<unsigned int, 3>& iP,
                             const T density, const MathVector<T, L::dimD>& velocity) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::GenericSRT>::setVariables")

      forcingScheme.setVariables(getForce(), density, velocity);
      equilibrium.setVariables(density,
                               forcingScheme.calculateEquilibriumVelocity(getForce()));
    }
  };


  template <class T>
  class Collision<T, CollisionType::BGK>
    : public Collision<T, CollisionType::GenericSRT> {

  protected:
    using Collision<T, CollisionType::GenericSRT>::tau;
    using Collision<T, CollisionType::GenericSRT>::equilibrium;
    using Collision<T, CollisionType::GenericSRT>::forcingScheme;

    T alpha;

  public:
    Collision(const T tau_in,
              const MathVector<T, 3>& amplitude_in,
              const MathVector<T, 3>& waveLength_in)
  : Collision<T, CollisionType::GenericSRT>(tau_in, amplitude_in, waveLength_in)
  , alpha( (T) 2)
    {}

    using Collision<T, CollisionType::GenericSRT>::setVariables;
    using Collision<T, CollisionType::GenericSRT>::setForce;

    using Collision<T, CollisionType::GenericSRT>::calculate;

    using Collision<T, CollisionType::GenericSRT>::getHydrodynamicVelocity;
    using Collision<T, CollisionType::GenericSRT>::getForce;

    DEVICE HOST
    inline T getAlpha() {
      return alpha;
    }
  };


  template <class T>
  class Collision<T, CollisionType::ELBM>
    : public Collision<T, CollisionType::BGK> {

  public:
    Collision(const T tau_in,
              const MathVector<T, 3>& amplitude_in,
              const MathVector<T, 3>& waveLength_in)
      : Collision<T, CollisionType::BGK>(tau_in, amplitude_in, waveLength_in)
      , beta( (T) 1.0/(2.0 * tau_in))
    {}

    using Collision<T, CollisionType::BGK>::setForce;

    #pragma omp declare simd
    DEVICE HOST
    inline void setVariables(const T * RESTRICT f,
                             const MathVector<unsigned int, 3>& iP,
                             const T density,
                             const MathVector<T, L::dimD>& velocity) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ELBM>::calculate")

      Collision<T, CollisionType::BGK>::setVariables(f, iP, density, velocity);

      UnrolledFor<0, L::dimQ>::Do([&] HOST DEVICE (unsigned int iQ) {
          f_Forced[iQ] = f[hD::getIndex(iP-uiL::celerity()[iQ], iQ)]
            + forcingScheme.getCollisionSource(getForce(), iQ);
          f_NonEq[iQ] = f[hD::getIndex(iP-uiL::celerity()[iQ], iQ)]
            - equilibrium.compute(iQ);
      });

      calculateAlpha();
      tau = (T) 1.0/(alpha*beta);
    }

    using Collision<T, CollisionType::BGK>::calculate;

    using Collision<T, CollisionType::BGK>::getHydrodynamicVelocity;
    using Collision<T, CollisionType::BGK>::getForce;

    using Collision<T, CollisionType::BGK>::getAlpha;

  protected:
    using Collision<T, CollisionType::BGK>::forcingScheme;
    using Collision<T, CollisionType::BGK>::equilibrium;

    using Collision<T, CollisionType::BGK>::tau;
    using Collision<T, CollisionType::BGK>::alpha;
    const T beta;

    MathVector<T, L::dimQ> f_Forced;
    MathVector<T, L::dimQ> f_NonEq;

    #pragma omp declare simd
    DEVICE HOST
    inline bool isDeviationSmall(const T error) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ELBM>::isDeviationSmall")

      bool isDeviationSmallR = true;
      T deviation;

      UnrolledFor<0, L::dimQ>::Do([&] HOST DEVICE (unsigned int iQ) {
          deviation = fabs(f_NonEq[iQ]/f_Forced[iQ]);

          if(deviation > error) {
            isDeviationSmallR = false;
          }
        });

      return isDeviationSmallR;
    }

    #pragma omp declare simd
    DEVICE HOST
    T calculateAlphaMax() {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ELBM>::calculateAlphaMax")

      T alphaMaxR = 2.5;
      T alphaMaxTemp;

      UnrolledFor<0, L::dimQ>::Do([&] HOST DEVICE (unsigned int iQ) {
          if(f_NonEq[iQ] > 0) {
            alphaMaxTemp = fabs(f_Forced[iQ]/f_NonEq[iQ]);

            if(alphaMaxTemp < alphaMaxR) {
              alphaMaxR = alphaMaxTemp;
            }
          }
        });

      return alphaMaxR;
    }

    #pragma omp declare simd
    DEVICE HOST
    inline T solveAlpha(const T alphaMin, const T alphaMax) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ELBM>::solveAlpha")

      std::shared_ptr<RootFinderFunctor<T>> entropicStepFunctor =
        std::shared_ptr<RootFinderFunctor<T>>(new EntropicStepFunctor<T>(f_Forced, f_NonEq));
      const T tolerance = 1e-5;
      const int iterationMax = 20;
      T alphaR = alpha;

      bool hasConverged = NewtonRaphsonSolver(entropicStepFunctor,
                                              tolerance, iterationMax,
                                              alphaR, alphaMin, alphaMax);

      if(!hasConverged) {
        return 2.0;
      }

      return alphaR;
    }

    #pragma omp declare simd
    DEVICE HOST
    inline void calculateAlpha() {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ELBM>::calculateAlpha")

      if(isDeviationSmall( (T) 1.0e-3)) {
        alpha = 2.0;
      }

      else {
        T alphaMax = calculateAlphaMax();

        if(alphaMax < 2.) {
          alpha = 0.95 * alphaMax;
        }

        else {
          T alphaMin = 1.;
          alpha = solveAlpha(alphaMin, alphaMax);

        }
      }
    }
  };


  template <class T>
  class Collision<T, CollisionType::Approached_ELBM>
    : public Collision<T, CollisionType::ELBM> {
  public:
    Collision(const T tau_in,
              const MathVector<T, 3>& amplitude_in,
              const MathVector<T, 3>& waveLength_in)
      : Collision<T, CollisionType::ELBM>(tau_in, amplitude_in, waveLength_in)
    {}

    using Collision<T, CollisionType::ELBM>::setForce;
    using Collision<T, CollisionType::ELBM>::setVariables;

    using Collision<T, CollisionType::ELBM>::calculate;

    using Collision<T, CollisionType::ELBM>::getHydrodynamicVelocity;
    using Collision<T, CollisionType::ELBM>::getForce;
    using Collision<T, CollisionType::ELBM>::getAlpha;

  private:
    using Collision<T, CollisionType::ELBM>::tau;
    using Collision<T, CollisionType::ELBM>::alpha;
    using Collision<T, CollisionType::ELBM>::forcingScheme;
    using Collision<T, CollisionType::ELBM>::equilibrium;
    using Collision<T, CollisionType::ELBM>::f_Forced;
    using Collision<T, CollisionType::ELBM>::f_NonEq;

    using Collision<T, CollisionType::ELBM>::isDeviationSmall;
    using Collision<T, CollisionType::ELBM>::calculateAlphaMax;
    using Collision<T, CollisionType::ELBM>::solveAlpha;

    #pragma omp declare simd
    DEVICE HOST
    inline T approximateAlpha() {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::Approached_ELBM>::approximateAlpha")

      T a1 = 0.0;
      T a2 = 0.0;
      T a3 = 0.0;
      T a4 = 0.0;

      UnrolledFor<0, L::dimQ>::Do([&] HOST DEVICE (unsigned int iQ) {
          T temp = f_NonEq[iQ]/f_Forced[iQ];
          a1 += f_NonEq[iQ]*temp;
          a2 += f_NonEq[iQ]*temp*temp;
          a3 += f_NonEq[iQ]*temp*temp*temp;
          a4 += f_NonEq[iQ]*temp*temp*temp*temp;
        });

      a1 *= 1.0/2.0;
      a2 *= 1.0/6.0;
      a3 *= 1.0/12.0;
      a4 *= 1.0/20.0;

      T alphaR = 2 - 1/a1 * (4.0*a2 + 16.0*a2*a2/a1 - 8.0*a3
                             + 80.0*a2*a3/a1 - 80.0*a2*a2*a2/(a1*a1) - 16.0*a4);

      return alphaR;
    }

    #pragma omp declare simd
    DEVICE HOST
    inline void calculateAlpha() {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::Approached_ELBM>::calculateAlpha")

      if(isRelativeDeviationSmall( (T) 1.0e-3)) {
        T alphaApproximated = approximateAlpha();
        alpha = alphaApproximated;
      }
      else {
        T alphaMax = calculateAlphaMax();

        if(alphaMax < 2.) {
          alpha = 0.95 * alphaMax;
        }

        else {
          T alphaMin = 1.;
          alpha = solveAlpha(alphaMin, alphaMax);

        }
      }
    }
  };


  template <class T>
  class Collision<T, CollisionType::ForcedNR_ELBM>
    : public Collision<T, CollisionType::ELBM> {
  public:
    using Collision<T, CollisionType::ELBM>::setForce;
    using Collision<T, CollisionType::ELBM>::setVariables;

    using Collision<T, CollisionType::ELBM>::calculate;

    using Collision<T, CollisionType::ELBM>::getHydrodynamicVelocity;
    using Collision<T, CollisionType::ELBM>::getForce;
    using Collision<T, CollisionType::ELBM>::getAlpha;

  private:
    using Collision<T, CollisionType::ELBM>::alpha;
    using Collision<T, CollisionType::ELBM>::forcingScheme;
    using Collision<T, CollisionType::ELBM>::equilibrium;

    using Collision<T, CollisionType::ELBM>::f_Forced;
    using Collision<T, CollisionType::ELBM>::f_NonEq;

    using Collision<T, CollisionType::ELBM>::isDeviationSmall;
    using Collision<T, CollisionType::ELBM>::calculateAlphaMax;

    #pragma omp declare simd
    DEVICE HOST
    inline T solveAlpha(const T alphaMin, const T alphaMax) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ForcedNR_ELBM>::solveAlpha")

      std::shared_ptr<RootFinderFunctor<T>> entropicStepFunctor =
        std::shared_ptr<RootFinderFunctor<T>>(new EntropicStepFunctor<T>(f_Forced, f_NonEq));
      const T tolerance = 1e-5;
      const int iterationMax = 20;
      T alphaR = alpha;

      bool hasConverged = NewtonRaphsonSolver(entropicStepFunctor, tolerance, iterationMax,
                                              alphaR, alphaMin, alphaMax);

      if(!hasConverged) {
        return 2.0;
      }

      return alphaR;
    }

    #pragma omp declare simd
    DEVICE HOST
    inline T calculateAlpha() {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ForcedNR_ELBM>::calculateAlpha")

      T alphaMax = calculateAlphaMax();

      if(alphaMax < 2.) {
        alpha = 0.95 * alphaMax;
      }

      else {
        T alphaMin = 1.;
        alpha = solveAlpha(alphaMin, alphaMax);
      }
    }
  };

  template <class T>
  class Collision<T, CollisionType::ForcedBNR_ELBM>
    : public Collision<T, CollisionType::ForcedNR_ELBM> {
  public:
    using Collision<T, CollisionType::ForcedNR_ELBM>::setForce;
    using Collision<T, CollisionType::ForcedNR_ELBM>::setVariables;

    using Collision<T, CollisionType::ForcedNR_ELBM>::calculate;

    using Collision<T, CollisionType::ForcedNR_ELBM>::getHydrodynamicVelocity;
    using Collision<T, CollisionType::ForcedNR_ELBM>::getForce;
    using Collision<T, CollisionType::ForcedNR_ELBM>::getAlpha;

  private:
    using Collision<T, CollisionType::ForcedNR_ELBM>::alpha;
    using Collision<T, CollisionType::ForcedNR_ELBM>::f_Forced;
    using Collision<T, CollisionType::ForcedNR_ELBM>::f_NonEq;


    using Collision<T, CollisionType::ForcedNR_ELBM>::forcingScheme;
    using Collision<T, CollisionType::ForcedNR_ELBM>::equilibrium;

    using Collision<T, CollisionType::ForcedNR_ELBM>::isDeviationSmall;
    using Collision<T, CollisionType::ForcedNR_ELBM>::calculateAlphaMax;
    using Collision<T, CollisionType::ForcedNR_ELBM>::calculateAlpha;

    #pragma omp declare simd
    DEVICE HOST
    inline T solveAlpha(const T alphaMin, const T alphaMax) {
      SCOREP_INSTRUMENT_OFF("Collision<T, CollisionType::ForcedBNR_ELBM>::solveAlpha")

      std::shared_ptr<RootFinderFunctor<T>> entropicStepFunctor =
        std::shared_ptr<RootFinderFunctor<T>>(new EntropicStepFunctor<T>(f_Forced, f_NonEq));
      const T tolerance = 1e-5;
      const int iterationMax = 20;
      T alphaR = alpha;

      bool hasConverged = Bisection_NewtonRaphsonSolver(entropicStepFunctor,
                                                        tolerance, iterationMax,
                                                        alphaR, alphaMin, alphaMax);

      if(!hasConverged) {
        return 2.0;
      }

      return alphaR;
    }
  };


  typedef Collision<dataT, collisionT> Collision_;

}

#endif // COLLISION_H

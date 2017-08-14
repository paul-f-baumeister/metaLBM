#ifndef BOUNDARY_H
#define BOUNDARY_H

#include "Input.h"
#include "Options.h"
#include "Domain.h"
#include "MathVector.h"
#include "StaticArray.h"

namespace lbm {

  template<class T, BoundaryType boundaryType>
  class Boundary{};

  template<class T>
  class Boundary<T, BoundaryType::Periodic> {
  private:
    MathVector<unsigned int, 3> iP_Origin;
    MathVector<unsigned int, 3> iP_Destination;

  protected:
    inline void applyX(T * __restrict__ f,
                       const MathVector<unsigned int, 3>& iP) {
      iP_Origin = {L::halo()[d::X], iP[d::Y], iP[d::Z]};
      iP_Destination = {L::halo()[d::X] + lD::length()[d::X], iP[d::Y], iP[d::Z]};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
      });

      iP_Origin = {L::halo()[d::X]+ lD::length()[d::X] -1, iP[d::Y], iP[d::Z]};
      iP_Destination = {0, iP[d::Y], iP[d::Z]};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
      });

    }

    inline void applyY(T * __restrict__ f,
                       const MathVector<unsigned int, 3>& iP) {
      iP_Origin = {iP[d::X], L::halo()[d::Y], iP[d::Z]};
      iP_Destination = {iP[d::X], L::halo()[d::Y] + lD::length()[d::Y], iP[d::Z]};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
        });

      iP_Origin = {iP[d::X], L::halo()[d::Y]+ lD::length()[d::Y] -1, iP[d::Z]};
      iP_Destination = {iP[d::X], 0, iP[d::Z]};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
        });
    }

    inline void applyZ(T * __restrict__ f,
                       const MathVector<unsigned int, 3>& iP) {
      iP_Origin = {iP[d::X], iP[d::Y], L::halo()[d::Z]};
      iP_Destination = {iP[d::X], iP[d::Y], L::halo()[d::Z] + lD::length()[d::Z]};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
      });

      iP_Origin = {iP[d::X], iP[d::Y], L::halo()[d::Z] + lD::length()[d::Z] - 1};
      iP_Destination = {iP[d::X], iP[d::Y], 0};

      UnrolledFor<0, L::dimQ>::Do([&] (int iQ) {
          f[hD::getIndex(iP_Destination, iQ)] = f[hD::getIndex(iP_Origin, iQ)];
      });

    }

  };

  template<class T>
  class Boundary<T, BoundaryType::Generic> {

  public:
    void apply() {}
  };

  template<class T>
  class Boundary<T, BoundaryType::BounceBack_Halfway>
    : public Boundary<T, BoundaryType::Generic> {
  protected:

  public:
    void apply() {}

  };

  template<class T>
  class Boundary<T, BoundaryType::Entropic>
    : public Boundary<T, BoundaryType::Generic> {
  protected:

  public:
    void apply() {}

  };


  typedef Boundary<dataT, boundaryT> Boundary_;

}

#endif // BOUNDARY_H

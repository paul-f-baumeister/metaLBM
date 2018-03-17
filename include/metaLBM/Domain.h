#ifndef DOMAIN_H
#define DOMAIN_H

#include "Options.h"
#include "Helpers.h"
#include "Lattice.h"
#include "MathVector.h"

namespace lbm {

  /**
   * Domain defining space where DynamicArray lives and providing them
   * with a multi-dimensional index.
   *
   * @tparam latticeType type of lattice used.
   * @tparam domainType type of domain locality used.
   * @tparam partitionningType type of domain partitionning used.
   * @tparam memoryLayout type of memory layout used.
   */

  template <DomainType domainType, PartitionningType partitionningType,
            MemoryLayout memoryLayout, unsigned int NumberComponents>
  struct Domain {};


  template <unsigned int NumberComponents>
  struct Domain<DomainType::LocalSpace, PartitionningType::Generic,
                MemoryLayout::Generic, NumberComponents> {
  public:
    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pStart() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pEnd() {
      return ProjectPadRealAndLeave1<unsigned int, L::dimD>::Do({{lengthX_g/NPROCS,
            lengthY_g, lengthZ_g}});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pLength() {
      return pEnd();
    }

    HOST DEVICE
    static inline unsigned int pVolume() {
      return pLength()[d::X]*pLength()[d::Y]*pLength()[d::Z];
    }


    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sStart() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sEnd() {
      return ProjectAndLeave1<unsigned int, L::dimD>::Do({{lengthX_g/NPROCS,
            lengthY_g, lengthZ_g}});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sLength() {
      return sEnd();
    }

    HOST DEVICE
    static inline unsigned int sVolume() {
      return sLength()[d::X]*sLength()[d::Y]*sLength()[d::Z];
    }

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP) {
      return pLength()[d::Z] * (pLength()[d::Y] * iP[d::X] + iP[d::Y]) + iP[d::Z];
    }

  };


  template <PartitionningType partitionningType, unsigned int NumberComponents>
  struct Domain<DomainType::GlobalSpace, partitionningType,
                MemoryLayout::Generic, NumberComponents>
    : public Domain<DomainType::LocalSpace, PartitionningType::Generic,
                    MemoryLayout::Generic, NumberComponents> {
  private:
    using Base = Domain<DomainType::LocalSpace, PartitionningType::Generic,
                        MemoryLayout::Generic, NumberComponents>;

  public:

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pStart() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pEnd() {
      return ProjectAndLeave1<unsigned int, L::dimD>::Do({{NPROCS * Base::pLength()[d::X], Base::pLength()[d::Y],Base::pLength()[d::Z]}});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> pLength() {
      return pEnd();
    }

    HOST DEVICE
    static inline unsigned int pVolume() {
      return pLength()[d::X]*pLength()[d::Y]*pLength()[d::Z];
    }

    HOST DEVICE
    static inline MathVector<unsigned int, 3> pOffset(const MathVector<int, 3>& rankMPI) {
      MathVector<unsigned int, 3> offsetR{{0}};
      for(unsigned int iD = 0; iD < L::dimD; ++iD) {
        offsetR[iD] = (unsigned int) Base::pLength()[iD] * rankMPI[iD];
      }
      return offsetR;
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sStart() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sEnd() {
      return ProjectAndLeave1<unsigned int, L::dimD>::Do({{lengthX_g, lengthY_g, lengthZ_g}});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> sLength() {
      return sEnd();
    }

    HOST DEVICE
    static inline unsigned int sVolume() {
      return sLength()[d::X]*sLength()[d::Y]*sLength()[d::Z];
    }

    HOST DEVICE
    static inline MathVector<unsigned int, 3> sOffset(const MathVector<int, 3>& rankMPI) {
      MathVector<unsigned int, 3> offsetR{{0}};
      for(unsigned int iD = 0; iD < L::dimD; ++iD) {
        offsetR[iD] = (unsigned int) Base::sLength()[iD] * rankMPI[iD];
      }
      return offsetR;
    }


    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP) {
      const unsigned int indexLocal = Base::getIndex({iP[d::X] - iP[d::X]/Base::length()[d::X] * Base::length()[d::X], iP[d::Y], iP[d::Z]});
      return iP[d::X]/Base::length()[d::X] * Base::volume() + indexLocal;
    }

  };


  template <>
  struct Domain<DomainType::HaloSpace, PartitionningType::Generic,
                MemoryLayout::Generic, L::dimQ>
    : public Domain<DomainType::LocalSpace, PartitionningType::Generic,
                    MemoryLayout::Generic, L::dimQ> {
  private:
    using Base = Domain<DomainType::LocalSpace, PartitionningType::Generic,
                        MemoryLayout::Generic, L::dimQ>;
  public:
    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> start() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> end() {
      return Base::sLength() + L::halo() + L::halo();
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> length() {
      return Base::sLength() + L::halo() + L::halo();
    }

    HOST DEVICE
    static inline unsigned int volume() {
      return length()[d::X]*length()[d::Y]*length()[d::Z];
    }

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP) {
      return length()[d::Z] * (length()[d::Y] * iP[d::X] + iP[d::Y]) + iP[d::Z];
    }

    HOST DEVICE
    static inline unsigned int getIndexLocal(const MathVector<unsigned int, 3>& iP) {
      return Base::getIndex(iP - L::halo());
    }

  };


  template <>
  struct Domain<DomainType::HaloSpace, PartitionningType::Generic,
                MemoryLayout::AoS, L::dimQ>
    : public Domain<DomainType::HaloSpace, PartitionningType::Generic,
                    MemoryLayout::Generic, L::dimQ> {
  private:
    using Base = Domain<DomainType::HaloSpace, PartitionningType::Generic,
                        MemoryLayout::Generic, L::dimQ>;

  public:
    using Base::start;
    using Base::end;
    using Base::length;
    using Base::volume;
    using Base::getIndex;
    using Base::getIndexLocal;

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP,
                                        const unsigned int iC) {
      return getIndex(iP) * L::dimQ + iC;
    }

    HOST DEVICE
    static inline unsigned int getIndex(const unsigned int index,
                                        const unsigned int iC) {
      return index * L::dimQ + iC;
    }

  };

  template <>
  struct Domain<DomainType::HaloSpace, PartitionningType::Generic,
                MemoryLayout::SoA, L::dimQ>
    : public Domain<DomainType::HaloSpace, PartitionningType::Generic,
                    MemoryLayout::Generic, L::dimQ> {
  private:
    using Base = Domain<DomainType::HaloSpace, PartitionningType::Generic,
                        MemoryLayout::Generic, L::dimQ>;

  public:
    using Base::start;
    using Base::end;
    using Base::length;
    using Base::volume;
    using Base::getIndex;
    using Base::getIndexLocal;

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP,
                                        const unsigned int iC) {
      return iC * volume() + getIndex(iP);
    }

    HOST DEVICE
    static inline unsigned int getIndex(const unsigned int index,
                                        const unsigned int iC) {
      return iC * volume() + index;
    }

  };


  template <unsigned int NumberComponents>
  struct Domain<DomainType::BufferXSpace, PartitionningType::Generic,
                MemoryLayout::Generic, NumberComponents>
    : public Domain<DomainType::HaloSpace, PartitionningType::Generic,
                    MemoryLayout::Generic, NumberComponents> {
  private:
    using Base = Domain<DomainType::HaloSpace, PartitionningType::Generic,
                        MemoryLayout::Generic, NumberComponents>;

  public:
    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> start() {
      return MathVector<unsigned int, 3>({0, 0, 0});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> end() {
      return MathVector<unsigned int, 3>({L::halo()[d::X],
            ProjectAndLeave1<unsigned int, L::dimD>::Do(Base::length())[d::Y],
            ProjectAndLeave1<unsigned int, L::dimD>::Do(Base::length())[d::Z]});
    }

    HOST DEVICE
    static inline constexpr MathVector<unsigned int, 3> length() {
      return end();
    }

    HOST DEVICE
      static inline unsigned int volume() {
      return length()[d::X]*length()[d::Y]*length()[d::Z];
    }

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP) {
      return length()[d::Z] * (length()[d::Y] * iP[d::X] + iP[d::Y]) + iP[d::Z];
    }

    HOST DEVICE
    static inline unsigned int getIndex(const MathVector<unsigned int, 3>& iP,
                                        const unsigned int iC) {
      return iC * volume() + getIndex(iP);
    }
  };

  typedef Domain<DomainType::GlobalSpace, PartitionningType::Generic,
                 MemoryLayout::Generic, 1> gSD;

  typedef Domain<DomainType::LocalSpace, PartitionningType::Generic,
                 MemoryLayout::Generic, 1> lSD;
  typedef Domain<DomainType::HaloSpace, PartitionningType::Generic,
                 memoryL, L::dimQ> hSD;
  typedef Domain<DomainType::BufferXSpace, PartitionningType::Generic,
                 MemoryLayout::Generic, L::dimQ> bXSD;

} // namespace lbm

#endif // DOMAIN_H

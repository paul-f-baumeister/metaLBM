#!/bin/bash
#
#SBATCH --job-name=lbm-24
#SBATCH --partition=NODE2011
#SBATCH --nodes=11
#SBATCH --ntasks-per-node=24
#SBATCH --nodelist=node[14,15,16,17,18,19,20,21,22,23,24]
#SBATCH --exclusive

CMAKEDIR=/home/tauzin/software/cmake
HDF5DIR=/home/tauzin/software/hdf5
FFTWDIR=/home/tauzin/software/fftw
BOOSTDIR=/home/tauzin/software/boost
MPIDIR=/home/tauzin/software/openmpi
GCCDIR=/home/tauzin/software/gcc

export PATH=$PATH:${CMAKEDIR}/bin:${MPIDIR}/bin:${GCCDIR}/bin
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${HDF5DIR}/lib:${FFTWDIR}/lib:${BOOSTDIR}/lib:${MPIDIR}/lib:${GCCDIR}/lib:$\
{GCCDIR}/lib64
export CC=${GCCDIR}/bin/gcc
export CXX=${GCCDIR}/bin/g++

export OMP_PROC_BIND=true
export OMP_PLACES=cores

cd $HOME/Workspace/lbm_solver/solver/bin
mpiexec -np $1 --map-by $2 -bind-to $2 ./lbm

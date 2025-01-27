#!/bin/bash
# - executes a test of a bcopy for a 4GB device [2GB bcopy]

PHYSRAND=/home/newton/newton/research/code/goblinCore/2HMCs/gc64-hmcsim/test/hmc_physrand/physrand

BANKS=16
CAPACITY=4
LINKS=4
BSIZE=64
QDEPTH=64
XDEPTH=128
VAULTS=32
NRQSTS=50
READ=50
WRITE=50
DRAMS=20

echo "Executing : $PHYSRAND -b $BANKS -c $CAPACITY -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -x $XDEPTH\
	-d $DRAMS -v $VAULTS -N $NRQSTS -R $READ -W $WRITE -S 65656"

$PHYSRAND -b $BANKS -c $CAPACITY -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -x $XDEPTH\
	-d $DRAMS -v $VAULTS -N $NRQSTS -R $READ -W $WRITE -S 65656

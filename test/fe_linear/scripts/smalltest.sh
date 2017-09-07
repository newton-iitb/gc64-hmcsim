#!/bin/bash

ulimit -s unlimited

MUTEX=../fe_linear
RUNDIR=`pwd`

BANKS=16
CAPACITY=4
LINKS=4
BSIZE=64
QDEPTH=64
XDEPTH=128
VAULTS=32
DRAMS=20
THREADS=2
LIBSRC=../../../cmc/fullempty/


echo "Executing $MUTEX -L $LIBSRC -b $BANKS -c $CAPACITY -d $DRAMS -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -v $VAULTS -x $XDEPTH -T $THREADS"


$MUTEX -L $LIBSRC -b $BANKS -c $CAPACITY -d $DRAMS -l $LINKS -m $BSIZE -n 1 -q $QDEPTH -v $VAULTS -x $XDEPTH -T $THREADS

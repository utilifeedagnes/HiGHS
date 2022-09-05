#!/usr/bin/env bash

BASEDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm -rf $BASEDIR/build
rm -rf $BASEDIR/install
mkdir $BASEDIR/build
cmake -E make_directory ${BASEDIR}/build
cmake -E make_directory ${BASEDIR}/install
cd $BASEDIR/build
echo $BASEDIR
echo "$PWD"
ls
cmake $BASEDIR -DCMAKE_INSTALL_PREFIX=${BASEDIR}/install/highs -DFAST_BUILD=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON

cmake --build . --parallel 
cmake --install .
source $1
export HIGHS_INSTALL=${BASEDIR}/install/highs/lib
cd $BASEDIR/src/interfaces/highspy
python -m build --wheel .
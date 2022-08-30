#!/usr/bin/env bash

BASEDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
venv_path = $1

rm -rf $BASEDIR/build
mkdir $BASEDIR/build
cmake -E make_directory ${BASEDIR}/build
cmake -E make_directory ${BASEDIR}/install
cd $BASEDIR/build
echo $BASEDIR
echo "$PWD"
ls
cmake $BASEDIR -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=${BASEDIR}/install/highs -DFAST_BUILD=ON
cmake --build . --parallel 
cmake --install .
source $1
export LD_LIBRARY_PATH=${BASEDIR}/install/highs/lib
pip install $BASEDIR/src/interfaces/highspy
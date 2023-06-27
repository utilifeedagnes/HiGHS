#!/usr/bin/env bash
cd $0
WHEEL_DIR=$PWD/$1
BASEDIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

rm -rf $BASEDIR/build
rm -rf $BASEDIR/install
mkdir $BASEDIR/build
cmake -E make_directory ${BASEDIR}/build
cmake -E make_directory ${BASEDIR}/install
cd $BASEDIR/build

cmake $BASEDIR -DCMAKE_INSTALL_PREFIX=${BASEDIR}/install/highs -DFAST_BUILD=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_ZLIB=ON -DBUILD_SHARED_LIBS=OFF
cmake --build . --parallel 
cmake --install .

export HIGHS_INSTALL=${BASEDIR}/install/highs/lib
cd $BASEDIR
python -m build --wheel --outdir=$WHEEL_DIR .
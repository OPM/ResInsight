#!/bin/bash

# Run in docker container
# docker run -e PLAT=manylinux2014_x86_64 -it quay.io/pypa/manylinux2014_x86_64

VERSION_TAG=${1:-""}

#export PYTHON27=/usr/bin/python2.7
export PYTHON35=/opt/python/cp35-cp35m/bin/python
export PYTHON36=/opt/python/cp36-cp36m/bin/python
export PYTHON37=/opt/python/cp37-cp37m/bin/python
export PYTHON38=/opt/python/cp38-cp38/bin/python

/bin/bash /tmp/opm-common/python/setup-docker-image.sh

cd /tmp/opm-common

# Delete the folder if it already exists
if [ -d build ]; then
  rm -rf build
fi

mkdir build && cd build

cmake3 -DPYTHON_EXECUTABLE=${PYTHON35} -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
-DOPM_ENABLE_PYTHON=ON -DOPM_PYTHON_PACKAGE_VERSION_TAG=${VERSION_TAG} ..

# make step is necessary until the generated ParserKeywords/*.hpp are generated in the Python step
make -j2

./setup-package.sh
${PYTHON35} -m auditwheel repair python/dist/*cp35*.whl

# Run setup-package.sh four times (Python 3.5, 3.6, 3.7 and 3.8)
cmake3 -DPYTHON_EXECUTABLE=${PYTHON36} -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
-DOPM_ENABLE_PYTHON=ON -DOPM_PYTHON_PACKAGE_VERSION_TAG=${VERSION_TAG} ..
./setup-package.sh
${PYTHON36} -m auditwheel repair python/dist/*cp36*.whl

cmake3 -DPYTHON_EXECUTABLE=${PYTHON37} -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
-DOPM_ENABLE_PYTHON=ON -DOPM_PYTHON_PACKAGE_VERSION_TAG=${VERSION_TAG} ..
./setup-package.sh
${PYTHON37} -m auditwheel repair python/dist/*cp37*.whl

cmake3 -DPYTHON_EXECUTABLE=${PYTHON38} -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
-DOPM_ENABLE_PYTHON=ON -DOPM_PYTHON_PACKAGE_VERSION_TAG=${VERSION_TAG} ..
./setup-package.sh
${PYTHON38} -m auditwheel repair python/dist/*cp38*.whl

#cmake3 -DPYTHON_EXECUTABLE=${PYTHON27} -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 \
#-DOPM_ENABLE_PYTHON=ON -DOPM_ENABLE_DYNAMIC_BOOST=OFF -DOPM_ENABLE_DYNAMIC_PYTHON_LINKING=OFF ..
#./setup-package.sh
#${PYTHON27} -m auditwheel repair python/dist/*cp27*.whl


# Example of upload
# /usr/bin/python3 -m twine upload --repository testpypi wheelhouse/*

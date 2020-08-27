#!/bin/bash

# Script to be run on a manylinux2014 docker image to complete it for OPM usage.
# i.e. docker run -i -t quay.io/pypa/manylinux2014_x86_64 < setup-docker-image.sh

# A ready made Docker image is available at Dockerhub:
# docker run -i -t lindkvis/manylinux2014_opm:latest

yum-config-manager --add-repo \
https://www.opm-project.org/package/opm.repo
yum install -y cmake3 ccache boost169-devel boost169-static
yum install -y blas-devel suitesparse-devel trilinos-openmpi-devel

#${PYTHON27} -m pip install pip --upgrade
#${PYTHON27} -m pip install wheel setuptools twine pytest-runner auditwheel
${PYTHON35} -m pip install pip --upgrade
${PYTHON35} -m pip install wheel setuptools twine pytest-runner auditwheel
${PYTHON36} -m pip install pip --upgrade
${PYTHON36} -m pip install wheel setuptools twine pytest-runner auditwheel
${PYTHON37} -m pip install pip --upgrade
${PYTHON37} -m pip install wheel setuptools twine pytest-runner auditwheel
${PYTHON38} -m pip install pip --upgrade
${PYTHON38} -m pip install wheel setuptools twine pytest-runner auditwheel


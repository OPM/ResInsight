#!/bin/bash

if [ $# -lt 2 ]; then
   echo "You need to provide two arguments:"
   echo "  1. The folder in which to clone and build grpc"
   echo "  2. The folder in which to install grpc"
   echo "Both folders need to be user writeable but will be created if they do not exist"
   echo "Example: "
   echo "   build_grpc.sh /tmp/grpc `pwd`/GrpcInstall"
   exit 1
fi

parentdir="$(dirname $1)"
if [ -d $1 ]; then
  if ! [ -w $1 ]; then
    echo "$1 exists and you don't have the permissions to write to it"
    exit 1
  fi
elif [ -d ${parentdir} ]; then
  if ! [ -w ${parentdir} ]; then
    echo "You don't have the permissions to write to ${parentdir}"
    exit 1
  fi
else
  echo "${parentdir} does not exist"
  exit 1
fi

if [ -d "$2" ]; then
  if ! [ -w "$2" ]; then
    echo "You don't have the permissions to write to $2"
    exit 1
  fi
else
  mkdir -p "$2" || { echo "Failed to create $2. Make sure you have write permissions." ; exit 1; }
fi


echo "Cloning GRPC repository to: $1"
echo "Installing to: $2"

git clone https://github.com/grpc/grpc.git $1
cd $1
git checkout v1.21.1
git submodule init
git submodule update
export PROTOBUF_CONFIG_OPTS="--prefix=$2"
make prefix=$2
echo "Installing GRPC to $2"
make install prefix=$2
cd third_party/protobuf
make install


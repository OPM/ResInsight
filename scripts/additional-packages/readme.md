# Workflow to build and distribute OpenSSL 3

In Qt6, OpenSSL 3 is used from version 6.5. On RHEL8 the only available system package is OpenSSL1.1. The authentication framwork in Qt (in a ResInsight build seen in install/plugins/tls/libqopensslbackend.so) use OpenSSL3. This document describes how to build OpenSSL using vcpkg and bundle required dynamic libraries in the install package of ResInsight.

1. Build and install ResInsight
2. Build a dynamic version of OpenSSL 3 using vcpkg. In the /build folder, execute the following
   ../ThirdParty/vcpkg/vcpkg install --triplet x64-linux-dynamic --x-manifest-root=../scripts/additional-packages --x-install-root=./vcpkg_installed
   Make sure that the libraries libcrypto.so/libssl.so are installed in /build/vcpkg_installed/lib64
3. Copy libcrypto.so/libssl.so to the lib64 folder in the install package for ResInsight
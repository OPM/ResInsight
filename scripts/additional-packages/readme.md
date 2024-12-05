# Workflow to build and distribute OpenSSL 3

From Qt 6.5, OpenSSL 3 is used for authentication workflows. On RHEL8 the only available system package is OpenSSL 1.1. The authentication framwork in Qt (in a ResInsight build used by install/plugins/tls/libqopensslbackend.so) requires OpenSSL3. This document describes how to build OpenSSL3 using vcpkg and bundle required dynamic libraries in the install package of ResInsight. When running ResInsight without these libraries, the following error message is displayed:

    qt.tlsbackend.ossl: Incompatible version of OpenSSL (built with OpenSSL >= 3.x, runtime version is < 3.x)
    qt.network.ssl: No functional TLS backend was found

1. Build and install ResInsight
2. Build a dynamic version of OpenSSL 3 using vcpkg. In the /build folder, execute the following:

    ../ThirdParty/vcpkg/vcpkg install --triplet x64-linux-dynamic --x-manifest-root=../scripts/additional-packages --x-install-root=./vcpkg_installed_custom

   Make sure that the libraries `libcrypto.so/libssl.so` are installed in `/build/vcpkg_installed/lib64`
3. Copy `libcrypto.so/libssl.so` to the `/lib64` folder in the install package for ResInsight
4. Successful install of these libraries will show the text "Use of SSL is available and supported" in the About dialog in ResInsight.

https://learn.microsoft.com/en-us/vcpkg/commands/common-options#manifest-root

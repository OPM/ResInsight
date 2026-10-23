# vcpkg Configuration

## Files

| File | Purpose |
|------|---------|
| `vcpkg-configuration.json` | Default registry configuration used by all builds |
| `vcpkg-configuration-rhel8.json` | RHEL8-specific registry configuration with a separate baseline |

## Overlay ports

`ThirdParty/vcpkg-overlay-ports/` holds local copies of upstream vcpkg ports
that need a patch the upstream port does not carry. Each configuration file
opts in to individual ports rather than the whole directory, because the two
baselines resolve different versions: pointing a profile at the parent
directory would silently override every port in it.

| Port | Used by | Reason |
|------|---------|--------|
| `snappy` | `vcpkg-configuration.json` | `snappy.cc` guards `<immintrin.h>` on `SNAPPY_HAVE_BMI2`/`SNAPPY_HAVE_X86_CRC32`, which the port turns off, but `MemCopy64()` uses AVX intrinsics whenever `__AVX__` is defined. Fails on the ubuntu-26.04 runner. [microsoft/vcpkg#51047](https://github.com/microsoft/vcpkg/issues/51047) |
| `thrift` | `vcpkg-configuration-rhel8.json` | `Mutex.h` uses `int64_t` without including `<cstdint>`, which GCC 15 no longer provides transitively |

When adding a port, copy it from `ThirdParty/vcpkg/ports/<name>`, add the patch
to `PATCHES` with a comment explaining why, bump `port-version`, and add the
port's path to the `overlay-ports` array of the configurations that need it.
Drop the overlay once upstream carries an equivalent fix.

## Changing the baseline

The `baseline` field in each configuration file pins the vcpkg registry to a specific commit of [microsoft/vcpkg](https://github.com/microsoft/vcpkg). Packages available at that commit are used for dependency resolution.

- Update `vcpkg-configuration.json` for the default (Ubuntu/Windows) builds.
- Update `vcpkg-configuration-rhel8.json` independently when a different baseline is needed for RHEL8 (Rocky Linux 8) compatibility — e.g. to match older system libraries (glibc 2.28, OpenSSL 1.1).

## RHEL8 CI usage

Selecting the RHEL8 configuration does not rename or overwrite the tracked
`vcpkg-configuration.json`. Instead, the root `CMakeLists.txt` accepts a
`RESINSIGHT_VCPKG_PROFILE` cache variable. When set, it copies `vcpkg.json`
and `vcpkg-configuration-<profile>.json` into a profile-specific directory
under the build tree and points vcpkg's `VCPKG_MANIFEST_DIR` at it, before
`project()` triggers the vcpkg toolchain file. This must happen before
`project()`: the toolchain file only defaults `VCPKG_MANIFEST_DIR` to
`CMAKE_SOURCE_DIR` when the variable is not already defined, and it reads
that value while processing `project()`.

The workflows `.github/workflows/rhel8-unit-tests.yml` and
`.github/workflows/rhel8-package.yml` (and `.github/docker/Dockerfile.rhel8`)
select the RHEL8 baseline by passing this flag to `cmake`:

```
cmake -S /src -B /src/cmakebuild -G Ninja \
  ... \
  -DRESINSIGHT_VCPKG_PROFILE=rhel8 \
  -DCMAKE_TOOLCHAIN_FILE=ThirdParty/vcpkg/scripts/buildsystems/vcpkg.cmake
```

This ensures vcpkg uses the RHEL8 baseline without affecting other CI runs,
and without ever mutating a tracked file in the source tree.

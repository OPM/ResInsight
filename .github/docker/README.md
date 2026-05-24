# ResInsight CI Docker Images

This directory contains the Dockerfiles for the prebuilt CI images used by the
Linux CI workflows. Each image bakes in everything that does not change on a
per-PR basis so CI runs only have to recompile the translation units that
actually changed.

| Dockerfile           | Image (per repo)                                | Consumer workflows                                            |
| -------------------- | ----------------------------------------------- | ------------------------------------------------------------- |
| `Dockerfile.rhel8`   | `ghcr.io/<repo>/ci-rhel8`                       | `rhel8-unit-tests.yml`                                        |
| `Dockerfile.ubuntu24`| `ghcr.io/<repo>/ci-ubuntu24`                    | `ResInsightWithCacheLinuxContainer.yml`, `copilot-setup-steps.yml` |

Both images follow the same multi-stage pattern:

1. A `base` stage with the toolchain(s), Qt, and `buildcache`.
2. One or more `vcpkg-builder` stages that populate per-compiler vcpkg binary
   caches.
3. One or more `build-warmup` stages that perform a full ResInsight compile
   at `/src -> /src/cmakebuild` and stash the resulting buildcache.
4. A `final` stage that assembles the toolchain plus the baked caches.

Each image is rebuilt nightly and pushed to GitHub Container Registry. The
image path is computed per repo as `ghcr.io/${GITHUB_REPOSITORY,,}/<name>` so
forks publish to their own namespaces.

---

## RHEL8 image (`Dockerfile.rhel8`)

### What's in the image

Built from `rockylinux:8` in four stages:

- **Toolchain** — `gcc-toolset-14`, a from-source `libstdc++exp` matched to the
  toolset's GCC, Qt 6.6.3 (via `aqtinstall`), Python 3.9, Ninja, CMake.
- **buildcache** — built from source against the image's own glibc (the
  upstream `buildcache-linux.tar.gz` binary is linked against glibc 2.29+ and
  crashes on Rocky 8's glibc 2.28). Sourced from the canonical GitLab repo
  `bits-n-bites/buildcache`. On PATH, so `CMakeLists.txt` auto-wires it as
  `CMAKE_CXX_COMPILER_LAUNCHER`.
- **vcpkg binary cache** at `/opt/vcpkg-cache` — populated by running a CMake
  configure against the ResInsight manifest, then frozen. Consumed read-only
  in CI.
- **Warm ResInsight buildcache** at `/opt/buildcache` — populated by a full
  `ResInsight-tests` compile at `/src -> /src/cmakebuild`. The CI workflow
  clones into the same `/src` path so buildcache hashes line up; mismatched
  source paths or build flags collapse the hit rate.

### Where the image lives

| Context                                | Image path                                          |
| -------------------------------------- | --------------------------------------------------- |
| Upstream (`OPM/ResInsight`)            | `ghcr.io/opm/resinsight/ci-rhel8`                   |
| Fork (e.g. `magnesj/ResInsight`)       | `ghcr.io/magnesj/resinsight/ci-rhel8`               |

Each push tags both `:latest` and a date tag `:YYYY-MM-DD`.

### Viewing the image

- **Web UI** — package page on GitHub:
  `https://github.com/<owner>/ResInsight/pkgs/container/resinsight%2Fci-rhel8`
- **CLI** — list versions and tags via the GitHub API:

  ```
  gh api -H 'Accept: application/vnd.github+json' \
    /users/<owner>/packages/container/resinsight%2Fci-rhel8/versions \
    --jq '.[] | {tags: .metadata.container.tags, updated_at: .updated_at}'
  ```

  (Use `/orgs/<org>/packages/...` instead of `/users/<owner>/...` for org-owned
  packages like the upstream `OPM` namespace.)

### Building the image

#### Via GitHub Actions (the supported path)

The image is built and pushed by `.github/workflows/build-rhel8-image.yml`:

- **Nightly** at 00:00 UTC (the unit test workflow runs at 02:00 UTC).
- **On push** to any of: `Dockerfile.rhel8`, `build-rhel8-image.yml`,
  `vcpkg.json`, `vcpkg-configuration-rhel8.json`, `.gitmodules`.
- **Manually** via `workflow_dispatch`:

  ```
  gh workflow run build-rhel8-image.yml --repo <owner>/ResInsight --ref <branch>
  ```

  The workflow short-circuits if an image tagged with today's UTC date is
  already in GHCR. To force a same-day rebuild (e.g. after iterating on the
  Dockerfile within a single day), pass `force_rebuild=true`:

  ```
  gh workflow run build-rhel8-image.yml --repo <owner>/ResInsight --ref <branch> \
    -f force_rebuild=true
  ```

Cold first build runs the full `ResInsight-tests` compile in the warmup stage
(~1.5–2 h). Incremental nightly rebuilds reuse the BuildKit `gha` cache and
finish much faster.

#### Building locally

Useful for validating Dockerfile changes before pushing to CI. From the repo
root:

```
docker buildx build \
  --progress=plain \
  --file .github/docker/Dockerfile.rhel8 \
  --tag resinsight-ci-rhel8:local \
  .
```

Notes:

- Requires Docker with BuildKit (default in Docker Desktop / Docker 23+).
  The Dockerfile uses `# syntax=docker/dockerfile:1` and a BuildKit cache
  mount for the warmup buildcache.
- The build context is ~860 MB even with `.dockerignore` excluding host build
  outputs — most of that is `ThirdParty/vcpkg` and other submodules.
- Cold build is ~1.5–2 h end-to-end on a typical workstation; the resulting
  image is ~5.3 GB.
- Tee the output to a log file so a crash of the launching shell does not
  lose progress (the build itself keeps running in the Docker daemon):

  ```
  docker buildx build ... 2>&1 | tee ri-rhel8-build.log
  ```

#### Pushing a local image to GHCR

Useful when iterating on Dockerfile changes on a fork before merging: push to
your own GHCR namespace and trigger `rhel8-unit-tests.yml` on the fork to
verify end-to-end.

1. Ensure your `gh` token has the `write:packages` scope:

   ```
   gh auth refresh -h github.com -s write:packages
   ```

2. Tag the local image for your fork's GHCR namespace and the desired tags:

   ```
   docker tag resinsight-ci-rhel8:local ghcr.io/<owner>/resinsight/ci-rhel8:latest
   docker tag resinsight-ci-rhel8:local ghcr.io/<owner>/resinsight/ci-rhel8:$(date -u +%Y-%m-%d)
   ```

3. Log Docker in to GHCR using the `gh` token and push:

   ```
   gh auth token | docker login ghcr.io -u <owner> --password-stdin
   docker push ghcr.io/<owner>/resinsight/ci-rhel8:latest
   docker push ghcr.io/<owner>/resinsight/ci-rhel8:$(date -u +%Y-%m-%d)
   ```

### How the image is consumed

`.github/workflows/rhel8-unit-tests.yml` runs the unit tests inside this
image. It resolves the image name from `${GITHUB_REPOSITORY,,}` so the same
workflow definition works on the upstream repo and on forks without
modification.

The workflow clones the source manually into `/src` (not into
`GITHUB_WORKSPACE`) because buildcache hashes the absolute source path — the
clone path has to match the warmup compile's `/src` for the cache to hit.
Unity build is disabled in both the warmup and the CI configure for the same
reason: with unity on, a single `.cpp` change invalidates the entire unity
blob's cache entry.

### Retention

`.github/workflows/cleanup-rhel8-image.yml` runs daily at 03:00 UTC and
deletes old dated tags from GHCR:

- The `:latest` tag is always kept (the consumer workflow pulls it).
- The 3 most recent versions are kept regardless of age (safety floor so a
  string of broken nightly builds can't strand CI without a working image).
- Of the rest, anything older than 5 days is deleted.

The retention window and dry-run mode are exposed as `workflow_dispatch`
inputs for ad-hoc invocation:

```
gh workflow run cleanup-rhel8-image.yml --repo <owner>/ResInsight \
  -f dry_run=true -f retention_days=7
```

Owner type (User vs Organization) is auto-detected so the workflow runs
unchanged on the upstream repo and on forks.

---

## Ubuntu 24.04 image (`Dockerfile.ubuntu24`)

### What's in the image

Built from `ubuntu:24.04` in six stages:

- **Toolchain** — both compilers used by the Linux legs of the cache workflow:
  `gcc-14` from Ubuntu's repos and `clang-19` from the upstream LLVM apt repo
  (`apt.llvm.org/noble/llvm-toolchain-noble-19`). Plus Python 3.12, Ninja,
  CMake, and the GL / Qt apt deps used by the existing non-containerized legs.
- **Qt 6.7.0** at `/opt/Qt/6.7.0/gcc_64`, installed via `aqtinstall` with the
  `qtnetworkauth` module. Same version as the cache workflow's Linux legs.
- **buildcache** — `v0.33.0` linux-amd64 binary release from the canonical
  GitLab repo (`bits-n-bites/buildcache`). The RHEL8 image builds from source
  because Rocky 8's glibc 2.28 is too old for the precompiled binary; Ubuntu
  24.04 ships glibc 2.39 so the binary loads directly. On PATH, so
  `CMakeLists.txt` auto-wires it as `CMAKE_CXX_COMPILER_LAUNCHER`.
- **vcpkg binary caches** — two of them, populated by independent stages that
  run in parallel under BuildKit:
  - `/opt/vcpkg-cache-gcc`   (built with `gcc-14`)
  - `/opt/vcpkg-cache-clang` (built with `clang-19`)
- **Warm ResInsight buildcaches** — two of them, each populated by a full
  ResInsight compile (default target — wider than `ResInsight-tests` alone)
  at `/src -> /src/cmakebuild`:
  - `/opt/buildcache-gcc`   (gcc-14, GRPC on)
  - `/opt/buildcache-clang` (clang-19, GRPC on)

The final stage's ENV targets the gcc-14 leg (`BUILDCACHE_DIR=/opt/buildcache-gcc`,
`VCPKG_DEFAULT_BINARY_CACHE=/opt/vcpkg-cache-gcc`) so the Copilot Coding Agent's
`copilot-setup-steps.yml` works without overrides. The clang consumer overrides
both env vars at the job level.

### Where the image lives

| Context                                | Image path                                          |
| -------------------------------------- | --------------------------------------------------- |
| Upstream (`OPM/ResInsight`)            | `ghcr.io/opm/resinsight/ci-ubuntu24`                |
| Fork (e.g. `magnesj/ResInsight`)       | `ghcr.io/magnesj/resinsight/ci-ubuntu24`            |

Each push tags both `:latest` and a date tag `:YYYY-MM-DD`.

### Viewing the image

- **Web UI**: `https://github.com/<owner>/ResInsight/pkgs/container/resinsight%2Fci-ubuntu24`
- **CLI**: same `gh api` recipe as the RHEL8 image above, with
  `ci-ubuntu24` in place of `ci-rhel8`.

### Building the image

#### Via GitHub Actions

`.github/workflows/build-ubuntu24-image.yml`:

- **Nightly** at 00:00 UTC (the containerized cache workflow runs at 01:00
  UTC).
- **On push** to any of: `Dockerfile.ubuntu24`, `build-ubuntu24-image.yml`,
  `vcpkg.json`, `vcpkg-configuration.json`, `.gitmodules`. (Note the default
  `vcpkg-configuration.json`, not the RHEL8 variant.)
- **Manually** via `workflow_dispatch`, optionally with `force_rebuild=true`
  to skip the "today's image already exists" guard:

  ```
  gh workflow run build-ubuntu24-image.yml --repo <owner>/ResInsight --ref <branch> \
    -f force_rebuild=true
  ```

Cold first build runs the full ResInsight compile **twice** (once per
compiler) — ~3–4 h end-to-end. The job timeout is 360 min. Incremental
nightly rebuilds reuse the BuildKit `gha` cache and the per-compiler
`ri-ubuntu24-buildcache-{gcc,clang}` cache mounts, and finish much faster.

#### Building locally

```
docker buildx build \
  --progress=plain \
  --file .github/docker/Dockerfile.ubuntu24 \
  --tag resinsight-ci-ubuntu24:local \
  .
```

Notes:

- BuildKit required (same as the RHEL8 image).
- Cold local build is ~3–4 h on a typical workstation; the resulting image is
  ~6–7 GB (two toolchains, two warm caches).
- BuildKit runs the gcc and clang `vcpkg-builder` and `build-warmup` stages in
  parallel where it can, so wall-clock time is less than 2× the per-compiler
  cost.

#### Pushing a local image to GHCR

Same procedure as the RHEL8 image above, with `ci-ubuntu24` in place of
`ci-rhel8`.

### How the image is consumed

Two workflows use the image:

- **`.github/workflows/ResInsightWithCacheLinuxContainer.yml`** — parallel
  companion to `ResInsightWithCache.yml`'s Linux legs. Runs both compilers in
  a matrix, each leg overriding `BUILDCACHE_DIR` and `VCPKG_BINARY_SOURCES`
  to its compiler's baked cache. Source is cloned manually into `/src`
  (matching the warmup path) and `defaults.run.working-directory` is set to
  `/src` so every step runs there.
- **`.github/workflows/copilot-setup-steps.yml`** — the GitHub Copilot Coding
  Agent's pre-flight setup. Uses the image's gcc-14 default ENV; the agent's
  first incremental build only recompiles what it edits because the warm
  buildcache is already populated.

In both cases, unity build is OFF (matches the warmup) and the absolute source
path is `/src` (matches the warmup) — drift on either collapses the cache hit
rate.

### Retention

`.github/workflows/cleanup-ubuntu24-image.yml` runs daily at 03:00 UTC with
the same rules as the RHEL8 cleanup: keep `:latest`, keep the 3 most recent
versions as a safety floor, delete anything older than 5 days. The
`workflow_dispatch` `dry_run` and `retention_days` inputs are exposed the
same way.

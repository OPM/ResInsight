# RHEL8 CI Docker Image

This directory contains the Dockerfile for the prebuilt CI image used by the
RHEL8 unit test workflow. The image bakes in everything that does not change
on a per-PR basis so CI runs only have to recompile the translation units that
actually changed.

## What's in the image

Built from `rockylinux:8` in four stages (see `Dockerfile.rhel8`):

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

## Where the images live

The image is published to GitHub Container Registry under the repository
namespace, computed by the workflows as
`ghcr.io/${GITHUB_REPOSITORY,,}/ci-rhel8`. In practice:

| Context                                | Image path                                          |
| -------------------------------------- | --------------------------------------------------- |
| Upstream (`OPM/ResInsight`)            | `ghcr.io/opm/resinsight/ci-rhel8`                   |
| Fork (e.g. `magnesj/ResInsight`)       | `ghcr.io/magnesj/resinsight/ci-rhel8`               |

Each push tags both `:latest` and a date tag `:YYYY-MM-DD`.

### Viewing the images

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

## Building the image

### Via GitHub Actions (the supported path)

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

### Building locally

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

### Pushing a local image to GHCR

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

## How the image is consumed

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

## Retention

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

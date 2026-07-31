# Cloud Service API -- Developer Python Setup

The Cloud Service API is a local FastAPI service that bridges cloud data into ResInsight. It lives in
the `scripts/ri-cloud-api` submodule, and ResInsight starts it after you sign in to Sumo:

```
<python> -m uvicorn ri_cloud_api.main:app --host 127.0.0.1 --port <free port>
```

`<python>` is the interpreter from *Preferences -> Scripting -> Python Executable Location*, the same
one used for `rips`. ResInsight installs nothing itself, so that environment has to be ready up
front. The service code always runs from its source folder -- a development build uses the submodule
in place, an installed build a copy of it under `CloudServiceApi` -- so only the dependencies are
strictly required in the environment.

## Prerequisites

- Python 3.11 or later, which is what `ri-cloud-api` requires.
- The submodule checked out: `git submodule update --init`.

## Python Environment

- **Komodo**: the release already provides an environment with the required packages. Point *Python
  Executable Location* at it and the service starts on its own; the rest of this document does not
  apply.
- **Custom**: install into the environment you already use for `rips`, or into a dedicated one that
  *Python Executable Location* points at. The next section covers both.

Use [uv](https://docs.astral.sh/uv/getting-started/installation/) and `uv sync` for an environment
dedicated to the service, either of the other two recipes when it is shared with `rips`. All three
need an environment to install into -- an existing one, or a new virtual environment:

```powershell
python -m venv C:\venvs\venvRiCloudApi
C:\venvs\venvRiCloudApi\Scripts\activate
```

On Linux, `source ~/venvs/venvRiCloudApi/bin/activate`. Then set *Python Executable Location* to
`C:\venvs\venvRiCloudApi\Scripts\python.exe`, or `~/venvs/venvRiCloudApi/bin/python`.

Keep the environment out of the repository so `git clean` does not take it, and out of the install
tree.

## Installing the packages

Three ways, all installing `ri-cloud-api` and its two workspace libraries with their dependencies:

| Command | Installs | Effect on the environment |
| --- | --- | --- |
| [`uv sync`](#1-using-uv-sync) | exact versions from `uv.lock` | removes anything not in the lock |
| [`uv pip install`](#2-using-uv-pip-install) | exact versions from `uv.lock` | only adds |
| [`pip install`](#3-using-pip) | version ranges from `pyproject.toml` | only adds |

Each runs from the folder holding `ri_cloud_api`, which differs between a development build and an
[installed build](#installed-builds). The recipes below write `<python>` for the environment's
interpreter -- the one from *Python Executable Location*, on Windows typically
`<venv>\Scripts\python.exe`, on Linux `<venv>/bin/python`.

Installing the three `ri-cloud-*` packages is optional: ResInsight prepends every `libs/*/src` to
`PYTHONPATH` and runs from the source folder regardless. Leaving them out keeps the environment
smaller, at the cost of setting `PYTHONPATH` yourself when
[running the service by hand](#running-the-service-by-hand). Each recipe ends with that variant.

### 1. Using uv sync

```powershell
cd ResInsight\scripts\ri-cloud-api        # development
cd <install prefix>\bin\CloudServiceApi   # installed

uv sync --active --no-dev --frozen
```

`--active` installs into the activated environment instead of creating a `.venv` beside the package.
`--no-dev` skips the `dev` group -- `ruff`, `mypy`, `pytest` -- which uv includes unless asked not
to; drop it when working on the API itself. `--frozen` installs what `uv.lock` pins without
rewriting it, so the source folder stays clean even when the lock is behind `pyproject.toml`. The
`ri-cloud-*` packages go in editable.

`uv sync` owns the environment: it removes anything not in `uv.lock`, so do not point `--active` at a
shared one. `--inexact` keeps those packages, but [`uv pip install`](#2-using-uv-pip-install) is the
better tool there. And if the interpreter does not satisfy `requires-python = ">=3.11"`, uv deletes
and recreates the environment rather than reporting an error.

**Dependencies only:** add `--no-install-workspace`.

### 2. Using uv pip install

```powershell
cd ResInsight\scripts\ri-cloud-api        # development
cd <install prefix>\bin\CloudServiceApi   # installed

uv export --frozen --no-dev --no-hashes --format requirements-txt -o <somewhere>\req.txt
uv pip install --python <python> -r <somewhere>\req.txt
```

**`uv pip install` only adds and upgrades; it never removes anything missing from `uv.lock`**, which
makes it the right tool for an environment shared with `rips` or your own tooling.

- The export lists the `ri-cloud-*` packages as paths relative to the package directory, so run the
  install from that same folder. They install editable.
- `--python` names the target interpreter. Without it uv picks the activated or discovered
  environment. The target needs no `pip` of its own.
- `--frozen` reads `uv.lock` without rewriting it. Write `req.txt` outside the source folder so it
  does not show up as an untracked change there.
- Drop `--no-dev` for `ruff`, `mypy` and `pytest`. Do not add `--exact`, which turns on pruning.

**Dependencies only:** add `--no-emit-workspace` to the export.

### 3. Using pip

The required packages are declared in the [ri-cloud-api](https://github.com/OPM/ri-cloud-api)
repository, which is the source of truth for the version ranges:
[`pyproject.toml`](https://github.com/OPM/ri-cloud-api/blob/main/pyproject.toml) for the service,
[`libs/services/pyproject.toml`](https://github.com/OPM/ri-cloud-api/blob/main/libs/services/pyproject.toml)
for the service layer, and
[`libs/core_utils/pyproject.toml`](https://github.com/OPM/ri-cloud-api/blob/main/libs/core_utils/pyproject.toml)
for the shared utilities. Rather than repeating that list, let pip read it -- with the environment
activated:

```powershell
cd ResInsight\scripts\ri-cloud-api        # development
cd <install prefix>\bin\CloudServiceApi   # installed

pip install ./libs/core_utils ./libs/services .
```

Without activating it, `<python> -m pip install ...` does the same.

The local projects must be given explicitly and in that order, since `ri-cloud-core-utils` and
`ri-cloud-services` are on no index. Installing `.` last brings in the third-party dependencies,
`uvicorn` with the `[standard]` extra included. Unlike the `uv` recipes, pip copies the packages into
`site-packages`, so source edits stop taking effect -- add `-e` to each path for editable installs.

**Dependencies only:** follow with `pip uninstall -y ri-cloud-api ri-cloud-services ri-cloud-core-utils`.
Uninstalling does not remove what a package pulled in, so the dependencies remain.

## Installed builds

`cmake --install` copies the whole submodule to `<install prefix>\bin\CloudServiceApi`, beside the
executable, and an installed ResInsight runs that copy rather than your checkout. It is a plain file
copy that keeps the layout the recipes need -- `ri_cloud_api`, `libs/<lib>/src`, `pyproject.toml`,
`uv.lock` -- with `.git*`, `__pycache__`, `.venv` and `.*_cache` excluded.

Installing ResInsight installs no Python packages, so run one of the recipes above from
`CloudServiceApi`. Two things to keep in mind, since an install tree is often read-only or replaced
by the next install:

- The submodule has to be checked out *before* the install runs, or `CloudServiceApi` ends up empty.
- Keep the environment outside the install tree, and do not drop `--active` or `--frozen` -- without
  them uv writes a `.venv` into the install folder and rewrites `uv.lock` there.

## Running the service by hand

With the environment activated, run from the same folder the recipes used -- the working directory
ResInsight uses:

```powershell
uvicorn ri_cloud_api.main:app --host 127.0.0.1 --port 8000
```

Without activating it, `<python> -m uvicorn ri_cloud_api.main:app ...` does the same.
<http://127.0.0.1:8000/alive> is the endpoint ResInsight polls; it should answer `{"status":"alive"}`.

With only the dependencies installed, the workspace libraries under `libs/<lib>/src` are not on
`sys.path`, and the service fails with `ModuleNotFoundError: No module named 'ri_cloud_services'`.
Add them first, colon separated on Linux:

```powershell
$env:PYTHONPATH = "libs\core_utils\src;libs\services\src"
```

ResInsight never needs this -- `RiaCloudApiService` sets it before launching, whichever way the
environment was prepared.

## Where ResInsight looks for the package

A folder containing `ri_cloud_api`, in this order:

1. `<ResInsight executable folder>/CloudServiceApi/`
2. Each entry in *Preferences -> Scripting -> Shared Script Folder(s)* (semicolon-separated), and a
   nested `ri-cloud-api` folder inside each

An installed build finds its own copy first without any configuration. A development build has no
`CloudServiceApi` beside the executable and falls through to the preference -- add
`<repo>/scripts/ri-cloud-api`, or the `scripts` folder above it.

The order matters: preferences live under one `Ceetron/ResInsight` key shared by every ResInsight on
the machine, so an entry added for a development build is visible to an installed build as well, and
probing the executable's own folder first keeps it from shadowing the shipped copy. The launch is
logged with the directory it settled on:

```
Cloud API service: launching '... -m uvicorn ri_cloud_api.main:app ...' (working directory '...')
```

## Keeping it up to date

Service code needs nothing -- it runs from the folder, so restarting the service picks it up. For an
installed build, re-running the install refreshes that folder. Dependencies need the install re-run
after `git submodule update` moves the submodule pointer, or after `pyproject.toml` or `uv.lock`
changes. Nothing checks automatically, and a dependency added since you last installed surfaces only
as an import error at startup, so check after updating:

```powershell
uv sync --active --no-dev --check
```

It compares the environment against `uv.lock`, lists what is missing and exits non-zero when the two
have diverged, without changing anything. Pass the same flags you installed with, or it reports the
packages you deliberately left out as missing.

Without uv, `<python> -m pip check` catches a missing dependency only when the `ri-cloud-*` packages
are installed, since they are what declare the requirements -- in a dependencies-only environment it
reports no problem even when the service cannot import.

## Troubleshooting

Service output is forwarded to the ResInsight log, prefixed `Cloud API service:`.

- `no Python executable configured` -- *Python Executable Location* is empty.
- `'ri_cloud_api' not found in any of: ...` -- submodule missing, or not reachable from *Shared
  Script Folder(s)*. The message lists every location probed. On an installed build it means
  `CloudServiceApi` is empty, because the submodule was not checked out when ResInsight was installed.
- `ModuleNotFoundError: No module named 'ri_cloud_services'` -- `PYTHONPATH` not set for a manual run.
- Any other `ModuleNotFoundError` -- dependencies not installed into the configured interpreter, or
  installed into a different one than *Python Executable Location* points at.
- The service starts but runs code you did not expect -- check the working directory in the launch
  log. An installed build always prefers its own `CloudServiceApi` over *Shared Script Folder(s)*.
- A `uv sync` or `uv pip install` that fails on a cached package -- `uv cache clean` empties uv's
  cache so the next command re-downloads. It discards cached downloads and build artifacts only,
  never installed environments; the cost is a slower next install.

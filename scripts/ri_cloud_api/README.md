# ResInsight Cloud API service

A localhost FastAPI/uvicorn service whose lifecycle is managed by ResInsight
(`RiaCloudApiService`): it is started when Sumo authentication succeeds, bound to an
auto-selected free port, health-checked on `/alive`, restarted if unresponsive, and
killed when ResInsight closes.

## Location

The package lives at `scripts/ri_cloud_api` in the repository root.

On install, the contents of this folder are copied to `<install folder>/CloudServiceApi`
(see the *Installation packaging Cloud Service API* section in the top-level
`CMakeLists.txt`).

`RiaCloudApiService::serviceWorkingDirectory` locates the package by probing, in order:

1. the configured **Shared Script Folder(s)**
   (**Preferences → Scripting → Shared Script Folder(s)**), and
2. `<application dir>/CloudServiceApi/`

and returning the first folder that contains a `ri_cloud_api` directory. For a
development build, point one of the shared script folders at the repository `scripts`
folder (e.g. `C:/Git/ResInsight/scripts`), since the executable lives in the build tree
and not next to the package.

## Deployment status

> **Target state:** this service is intended to be distributed and installed as a
> regular **pip package** into the active Python environment. Once packaged, importing
> `ri_cloud_api` (and any `ri_cloud_*` workspace libraries) will work directly from the
> environment's site-packages, with no path manipulation.
>
> **Current state (interim):** the service is **not yet a pip package**. It runs from the
> source tree inside a virtual environment, with the folder containing `ri_cloud_api` used
> as the process working directory. If workspace libraries are laid out as
> `ri_cloud_api/libs/<lib>/src`, ResInsight adds each such `src` folder to `PYTHONPATH` at
> launch time (see `RiaCloudApiService::buildProcessEnvironment`). This is a temporary
> measure that should be removed once the package is published/installed.

## Requirements (interim setup)

- A Python virtual environment configured in ResInsight under
  **Preferences → Scripting → Python Executable**
  (e.g. `C:/Git/.venvResInsight/Scripts/python.exe`).
- `uvicorn` and `fastapi` installed in that environment, plus any third-party
  dependencies pulled in by the routers.

## Contents

    ri_cloud_api/
      main.py                       shim exposing `app` for `uvicorn ri_cloud_api.main:app`
      primary/
        main.py                     FastAPI application, router registration
        routers/health/router.py    `/alive` health endpoint
        utils/router_headers.py     shared router header helpers

## Running manually (for debugging)

From the `scripts` folder (the parent of this package):

    cd scripts
    <venv>/Scripts/python.exe -m uvicorn ri_cloud_api.main:app --host 127.0.0.1 --port 8000

If workspace libraries are present, put them on `PYTHONPATH` first:

    set PYTHONPATH=ri_cloud_api/libs/<lib>/src

Then check the health endpoint:

    http://127.0.0.1:8000/alive   ->   {"status": "alive"}

Interactive API docs are available at:

    http://127.0.0.1:8000/docs

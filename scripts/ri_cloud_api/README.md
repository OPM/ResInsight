# ResInsight Cloud API service

A localhost FastAPI/uvicorn service whose lifecycle is managed by ResInsight
(`RiaCloudApiService`): it is started when Sumo authentication succeeds, bound to an
auto-selected free port, health-checked on `/alive`, restarted if unresponsive, and
killed when ResInsight closes.

## Deployment status

> **Target state:** this service is intended to be distributed and installed as a
> regular **pip package** into the active Python environment. Once packaged, importing
> `ri_cloud_api` (and its `ri_cloud_*` workspace libraries) will work directly from the
> environment's site-packages, with no path manipulation.
>
> **Current state (interim):** the service is **not yet a pip package**. It runs from the
> source tree inside a virtual environment, and ResInsight makes the local workspace
> libraries importable by adding each `ri_cloud_api/libs/<lib>/src` folder to `PYTHONPATH`
> at launch time (see `RiaCloudApiService::buildProcessEnvironment`). This is a temporary
> measure that should be removed once the package is published/installed.

## Requirements (interim setup)

- A Python virtual environment configured in ResInsight under
  **Preferences → Scripting → Python Executable**
  (e.g. `D:/Git/.venvResInsight/Scripts/python.exe`).
- The following packages installed in that environment: `uvicorn`, `fastapi`, and the
  third-party dependencies pulled in by the routers (e.g. the Sumo access libraries).

## Running manually (for debugging)

From the `PythonExamples` folder, with the workspace libs on `PYTHONPATH`:

    cd GrpcInterface/Python/rips/PythonExamples
    set PYTHONPATH=ri_cloud_api/libs/services/src;ri_cloud_api/libs/core_utils/src
    <venv>/Scripts/python.exe -m uvicorn ri_cloud_api.main:app --host 127.0.0.1 --port 8000

Then check the health endpoint:

    http://127.0.0.1:8000/alive   ->   {"status": "alive"}

"""ResInsight Cloud API — FastAPI application.

Lifecycle is managed by ResInsight (RiaCloudApiService): the service is started when Sumo
authentication succeeds, bound to an auto-selected free port, health-checked on /alive,
restarted if unresponsive, and killed when ResInsight closes.

Deployment status:
    Target state — distributed and installed as a regular pip package, importable directly
    from the environment's site-packages.
    Current state (interim) — runs from the source tree in a virtual environment; ResInsight
    injects each ri_cloud_api/libs/<lib>/src folder onto PYTHONPATH at launch time. This is
    temporary and should be removed once the package is published/installed.

See README.md (next to this package) for details.

Setup:
    pip install poetry
    python -m venv .venv
    source .venv/bin/activate
    poetry install

Run (from the repository root):
    uvicorn ri_cloud_api.primary.main:app --host 0.0.0.0 --port 8000 --reload

Docs:
    http://localhost:8000/docs
"""

from __future__ import annotations

import asyncio
import logging
from contextlib import asynccontextmanager

from fastapi import FastAPI

from .utils.exception_handlers import add_exception_handlers
from .utils.parent_watchdog import watch_parent
from .routers.health.router import router as health_router
from .routers.explore.router import router as explore_router
from .routers.polygons.router import router as polygons_router
from .routers.surfaces.router import router as surfaces_router
from .routers.timeseries.router import router as timeseries_router
from .routers.grids.router import router as grids_router

logger = logging.getLogger("ri_cloud_api")
logging.basicConfig(level=logging.INFO)


@asynccontextmanager
async def lifespan(app: FastAPI):
    # Start the parent-PID watchdog so the service self-terminates if ResInsight crashes.
    watchdog_task = asyncio.create_task(watch_parent())
    try:
        yield
    finally:
        watchdog_task.cancel()


app = FastAPI(title="ResInsight Cloud API", lifespan=lifespan)

add_exception_handlers(app)

app.include_router(health_router)

app.include_router(explore_router)
app.include_router(timeseries_router)
app.include_router(polygons_router)
app.include_router(surfaces_router)
app.include_router(grids_router)
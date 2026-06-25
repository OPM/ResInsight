"""ResInsight Cloud API — FastAPI application.

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

import logging

from fastapi import FastAPI

from .utils.exception_handlers import add_exception_handlers
from .routers.explore.router import router as explore_router
from .routers.polygons.router import router as polygons_router
from .routers.surfaces.router import router as surfaces_router
from .routers.timeseries.router import router as timeseries_router
from .routers.grids.router import router as grids_router

logger = logging.getLogger("ri_cloud_api")
logging.basicConfig(level=logging.INFO)


app = FastAPI(title="ResInsight Cloud API")

add_exception_handlers(app)

app.include_router(explore_router)
app.include_router(timeseries_router)
app.include_router(polygons_router)
app.include_router(surfaces_router)
app.include_router(grids_router)
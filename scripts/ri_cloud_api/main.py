"""ResInsight Cloud API entrypoint.

Thin shim so ``uvicorn ri_cloud_api.main:app`` keeps working alongside the
real application module at :mod:`ri_cloud_api.primary.main`.
"""

from __future__ import annotations

from .primary.main import app

__all__ = ["app"]

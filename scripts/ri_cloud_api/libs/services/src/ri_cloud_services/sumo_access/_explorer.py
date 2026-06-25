"""Shared Sumo Explorer wiring.

A single cached Explorer instance is reused across requests so we don't
re-authenticate on every call. The case-lookup helper centralizes error
handling so individual accessors don't need to repeat it.
"""

from __future__ import annotations

import logging
import os

from fmu.sumo.explorer import Explorer
from fmu.sumo.explorer.objects import Case
from ri_cloud_services.service_exceptions import NoDataError, Service

logger = logging.getLogger("ri_cloud_api.sumo_access")

SUMO_ENV = os.environ.get("SUMO_ENV", "prod")


def get_explorer(access_token: str) -> Explorer:
    """Return a process-wide cached Explorer instance."""

    # TODO: ONLY PRINT TOKEN IN DEBUG MODE
    logger.info("Creating fmu-sumo Explorer (env=%s) with token=%s", SUMO_ENV, access_token)

    return Explorer(env=SUMO_ENV, token=access_token)


def get_case_by_uuid(access_token: str, case_uuid: str) -> Case:
    """Look up a Sumo case by uuid.

    Raises NoDataError if the case cannot be found; routers translate this
    into an HTTP 404.
    """
    try:
        return get_explorer(access_token).get_case_by_uuid(case_uuid)
    except Exception as exc:  # fmu-sumo raises a variety of error types
        raise NoDataError(f"Case '{case_uuid}' not found: {exc}", Service.SUMO) from exc

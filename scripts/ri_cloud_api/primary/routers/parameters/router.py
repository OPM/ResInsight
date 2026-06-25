"""Parameters router.

Exposes endpoints for discovering and (eventually) fetching parameters
data from Sumo. All Sumo Explorer interactions are delegated
to ``ParameterAccess`` in the service layer.
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Path

from ri_cloud_services.sumo_access.parameter_access import ParameterAccess

router = APIRouter(tags=["parameters"])


@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/parameters/blob_url")
async def get_parameters_blob_url(
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
) -> str:
    """Get the blob URL for the parameters table for the given case + ensemble"""
    access = ParameterAccess.from_case_uuid(case_uuid, ensemble_name)
    try:
        url = await access.get_parameters_blob_url_async()
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return url

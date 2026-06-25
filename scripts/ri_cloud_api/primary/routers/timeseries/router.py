"""Timeseries / summary router.

Exposes endpoints for discovering and (eventually) fetching summary
timeseries data from Sumo. All Sumo Explorer interactions are delegated
to ``SummaryAccess`` in the service layer.
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Header, Path

from ri_cloud_services.sumo_access.summary_access import SummaryAccess

from ri_cloud_api.primary.utils.router_headers import extract_required_token

from .schemas import VectorInfo

router = APIRouter(tags=["timeseries"])


@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/vector_list")
async def get_vector_list(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
) -> list[VectorInfo]:
    """List available summary vector names for the given case + ensemble."""
    access_token = extract_required_token(authorization)
    
    access = SummaryAccess.from_case_uuid(access_token, case_uuid, ensemble_name)
    try:
        names = await access.get_available_vectors_async()
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return [VectorInfo(name=n) for n in names]

@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/vectors/{vector_name}/blob_url")
async def get_vector_blob_url(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
    vector_name: str = Path(description="Vector name"),
) -> str:
    """Get the blob URL for the given summary vector"""
    access_token = extract_required_token(authorization)
    
    access = SummaryAccess.from_case_uuid(access_token, case_uuid, ensemble_name)
    try:
        url = await access.get_vector_blob_url_async(vector_name)
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return url

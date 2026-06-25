"""Grids router

Exposes endpoints for discovering and (eventually) fetching grid data from Sumo.
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Header, Path, Query

from ri_cloud_services.sumo_access.grid_access import GridAccess

from ri_cloud_api.primary.utils.router_headers import extract_required_token

from .schemas import GridInfo, GridPropertyInfo

router = APIRouter(tags=["grids"])

@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/grid_info_list")
async def get_grid_info_list(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name")
) -> list[GridInfo]:
    """List available grids, with their realizations, for the given case + ensemble."""
    access_token = extract_required_token(authorization)
    access = GridAccess.from_case_uuid(access_token, case_uuid, ensemble_name)

    try:
        grids = await access.get_available_grid_info_list_async()
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return [GridInfo(name=g.name, realizations=g.realizations) for g in grids]

@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/grids/{grid_name}/realizations/{realization}/blob_url")
async def get_grid_blob_url(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
    grid_name: str = Path(description="Grid name"),
    realization: int = Path(description="Realization id"),
) -> str:
    """Get the blob URL for the grid data for the given case + ensemble."""
    access_token = extract_required_token(authorization)
    access = GridAccess.from_case_uuid(access_token, case_uuid, ensemble_name)
    try:
        url = await access.get_grid_blob_url_async(grid_name, realization)
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return url


@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/grids/{grid_name}/realizations/{realization}/property_info_list")
async def get_grid_property_info_list(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
    grid_name: str = Path(description="Grid name"),
    realization: int = Path(description="Realization id"),
) -> list[GridPropertyInfo]:
    """Get grid property metadata for the given case + ensemble + grid + realization."""
    access_token = extract_required_token(authorization)
    access = GridAccess.from_case_uuid(access_token, case_uuid, ensemble_name)
    try:
        properties = await access.get_grid_properties_async(grid_name, realization)
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return [
        GridPropertyInfo(
            propertyName=prop.property_name,
            isoDateOrInterval=prop.iso_date_or_interval,
        )
        for prop in properties
    ]

@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/grids/{grid_name}/realizations/{realization}/properties/{property_name}/blob_url")
async def get_grid_property_blob_url(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
    grid_name: str = Path(description="Grid name"),
    realization: int = Path(description="Realization id"),
    property_name: str = Path(description="Property name"),
    property_iso_date_or_interval: str | None = Query(
        default=None, description="Time point or time interval string"
    ),
) -> str:
    """Get the blob URL for a grid property."""
    access_token = extract_required_token(authorization)
    access = GridAccess.from_case_uuid(access_token, case_uuid, ensemble_name)
    try:
        url = await access.get_grid_property_blob_url_async(grid_name, realization, property_name, property_iso_date_or_interval)
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return url
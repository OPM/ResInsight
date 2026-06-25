"""Explore router.

Exposes asset/case/ensemble discovery endpoints. All Sumo Explorer
interactions are delegated to ``CaseInventoryAccess`` in the service layer.
"""

from __future__ import annotations

from fastapi import APIRouter, HTTPException, Header, Path, Query

from ri_cloud_services.sumo_access.case_inventory_access import CaseInventoryAccess

from ...utils.router_headers import extract_required_token

from .schemas import AssetInfo, CaseInfo, EnsembleInfo

router = APIRouter(tags=["explore"])


def _access() -> CaseInventoryAccess:
    return CaseInventoryAccess()


@router.get("/assets")
def get_assets(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
) -> list[AssetInfo]:
    """List available Sumo assets."""
    access_token = extract_required_token(authorization)

    return [AssetInfo(name=n) for n in _access().get_asset_names(access_token)]


@router.get("/cases")
def get_cases(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    asset_name: str = Query(min_length=1, description="Asset name"),
) -> list[CaseInfo]:
    """List Sumo cases for a given asset."""
    access_token = extract_required_token(authorization)
    cases = _access().get_cases_for_asset(access_token, asset_name)
    return [
        CaseInfo(
            id=c.id,
            name=c.name,
            asset=c.asset,
            field=c.field,
            status=c.status,
            user=c.user,
        )
        for c in cases
    ]


@router.get("/cases/{case_uuid}/ensembles")
def get_ensembles(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
) -> list[EnsembleInfo]:
    """List ensembles for a case."""
    try:
        access_token = extract_required_token(authorization)
        names = _access().get_ensemble_names(access_token, case_uuid)
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc
    return [EnsembleInfo(name=n) for n in names]


@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/realizations")
async def get_ensemble_realizations(
    authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),
    case_uuid: str = Path(description="Sumo case uuid"),
    ensemble_name: str = Path(description="Ensemble name"),
) -> list[int]:
    """List realization ids for a case + ensemble."""
    try:
        access_token = extract_required_token(authorization)
        return await _access().get_ensemble_realization_ids_async(
            access_token, case_uuid, ensemble_name
        )
    except LookupError as exc:
        raise HTTPException(status_code=404, detail=str(exc)) from exc

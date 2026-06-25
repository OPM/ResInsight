"""Surfaces router.

Placeholder router; endpoints to be added once we wire up a
``SurfacesAccess`` accessor in the service layer.
"""

from __future__ import annotations

from fastapi import APIRouter, Header

router = APIRouter(tags=["surfaces"])


@router.get("/cases/{case_uuid}/ensembles/{ensemble_name}/surfaces")
def get_surfaces(
	case_uuid: str,
	ensemble_name: str,
	authorization: str | None = Header(None, description="Authorization bearer token for Sumo API"),	
) -> list[dict[str, str]]:
	"""Placeholder surfaces endpoint until SurfacesAccess is implemented."""
	_ = (authorization, case_uuid, ensemble_name)
	return []

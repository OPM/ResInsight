"""Access class for listing Sumo assets, cases and ensembles."""

from __future__ import annotations

from dataclasses import dataclass

from ._explorer import get_case_by_uuid, get_explorer


@dataclass(frozen=True)
class CaseSummary:
    id: str
    name: str
    asset: str | None
    field: str | None
    status: str | None
    user: str | None


class CaseInventoryAccess:
    """Read-only access to the case inventory in Sumo."""

    def get_asset_names(self, access_token: str | None) -> list[str]:
        return list(get_explorer(access_token).asset_names)

    def get_cases_for_asset(self, access_token:str, asset_name: str) -> list[CaseSummary]:
        cases = get_explorer(access_token).cases.filter(asset=asset_name)
        return [
            CaseSummary(
                id=c.uuid,
                name=c.name,
                asset=getattr(c, "asset", None),
                field=getattr(c, "field", None),
                status=getattr(c, "status", None),
                user=getattr(c, "user", None),
            )
            for c in cases
        ]

    def get_ensemble_names(self, access_token: str, case_uuid: str) -> list[str]:
        case = get_case_by_uuid(access_token, case_uuid)
        return list(case.ensembles.ensemblenames)

    async def get_ensemble_realization_ids_async(
        self, access_token: str, case_uuid: str, ensemble_name: str
    ) -> list[int]:
        case = get_case_by_uuid(access_token, case_uuid)
        ensemble = case.filter(ensemble=ensemble_name, realization=True)
        realization_ids = await ensemble.realizationids_async
        return sorted(int(r) for r in realization_ids)

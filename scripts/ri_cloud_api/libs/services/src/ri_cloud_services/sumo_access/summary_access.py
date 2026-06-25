"""Access class for summary / timeseries data on Sumo.

Wraps fmu-sumo-explorer summary tables so the router layer only has to
deal with simple, typed return values.
"""

from __future__ import annotations

from ri_cloud_services.service_exceptions import MultipleDataMatchesError, NoDataError, Service

from ._explorer import get_case_by_uuid


# Non-vector columns that may appear in a summary table and should be filtered
# out when listing available vectors.
_SUMMARY_METADATA_COLUMNS = {"DATE", "REAL", "ENSEMBLE", "ITER"}


class SummaryAccess:
    """Access summary (timeseries) data for a given Sumo case + ensemble."""

    def __init__(self, access_token: str, case_uuid: str, ensemble_name: str) -> None:
        self._access_token = access_token
        self._case_uuid = case_uuid
        self._ensemble_name = ensemble_name

    @classmethod
    def from_case_uuid(cls, access_token: str, case_uuid: str, ensemble_name: str) -> "SummaryAccess":
        return cls(access_token=access_token, case_uuid=case_uuid, ensemble_name=ensemble_name)

    async def get_available_vectors_async(self) -> list[str]:
        """Return the list of available summary vector names.

        Uses the summary table associated with the ensemble. Metadata
        columns (DATE, REAL, ENSEMBLE, ITER) are filtered out.
        """
        case = get_case_by_uuid(self._access_token, self._case_uuid)

        table_context = case.tables.filter(ensemble=self._ensemble_name, standard_result="simulationtimeseries")
        # table_context = case.tables.filter(
        #     ensemble=self._ensemble_name,
        #     tagname="summary",
        # )  

        if await table_context.length_async() == 0:
            raise NoDataError(
                f"No summary tables found for ensemble '{self._ensemble_name}' "
                f"in case '{self._case_uuid}'",
                Service.SUMO
            )
        
        table_names = await table_context.names_async
        if len(table_names) == 0:
            raise NoDataError(
                f"No summary tables found in case={self._case_uuid}, ensemble={self._ensemble_name}",
                Service.SUMO
            )
        if len(table_names) > 1:
            raise MultipleDataMatchesError(
                f"Multiple summary tables found in case={self._case_uuid}, ensemble={self._ensemble_name}: {table_names=}",
                Service.SUMO
            )
        
        column_names = await table_context.columns_async

        # Get set of columns names, not among ["YEARS", "DATE", "REAL"]
        vector_names = list(set(column_names) - _SUMMARY_METADATA_COLUMNS)
        return vector_names

    async def get_vector_blob_url_async(self, vector_name: str) -> str:
        """Get the blob URL for the given summary vector.

        The temporary solution is not optimized, so we trigger aggregation to ensure the blob URL is available, this triggers an aggregation 

        Returns the raw Azure blob URL. The caller should authenticate using
        OAuth Bearer token (same token used for Sumo API access).
        """

        case = get_case_by_uuid(self._access_token, self._case_uuid)

        # TODO: Ensure only one table name?
        sc_per_real_tables = case.tables.filter(
            ensemble=self._ensemble_name,
            column=vector_name,
            standard_result="simulationtimeseries", # TODO: Use standard_result type from fmu-data-io?
            # tagname="summary",
            realization=True
        )

        if len(sc_per_real_tables.names) == 0:
            raise NoDataError(
                f"No tables found for vector '{vector_name}' in case='{self._case_uuid}', ensemble='{self._ensemble_name}'",
                Service.SUMO
            )
        if len(sc_per_real_tables.names) > 1:
            raise MultipleDataMatchesError(
                f"Multiple tables found for vector '{vector_name}' in case='{self._case_uuid}', ensemble='{self._ensemble_name}': {sc_per_real_tables.names}",
                Service.SUMO
            )

        # Trigger aggregation if not existing
        agg_table = await sc_per_real_tables.aggregation_async(
            operation="collection",
            column=vector_name
        )

        blob_url = agg_table.metadata["_sumo"]["blob_url"]

        print(f"DEBUG: Blob URL for vector '{vector_name}': {blob_url}")

        return blob_url

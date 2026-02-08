"""
Sumo Explorer client wrapper

Wraps the fmu.sumo.explorer.Explorer API for easier access from FastAPI.
"""

import logging
from typing import List, Optional
import pandas as pd
import io

try:
    from fmu.sumo.explorer import Explorer
except ImportError:
    Explorer = None

from .models import Asset, Case, Ensemble, VectorInfo, RealizationInfo

logger = logging.getLogger(__name__)


class SumoClientWrapper:
    """Wrapper around Sumo Explorer API"""

    def __init__(self, environment: str = "prod"):
        """
        Initialize Sumo Explorer client

        Args:
            environment: Sumo environment to connect to (default: "prod")
        """
        self.environment = environment
        self._explorer: Optional[Explorer] = None
        self._connected = False
        self._error: Optional[str] = None

        if Explorer is None:
            self._error = "fmu-sumo package not installed"
            logger.error(self._error)
            return

        try:
            self._explorer = Explorer(env=environment)
            self._connected = True
            logger.info(f"Connected to Sumo environment: {environment}")
        except Exception as e:
            self._error = f"Failed to connect to Sumo: {str(e)}"
            logger.error(self._error)

    @property
    def is_connected(self) -> bool:
        """Check if connected to Sumo"""
        return self._connected

    @property
    def error(self) -> Optional[str]:
        """Get connection error if any"""
        return self._error

    def get_assets(self) -> List[Asset]:
        """
        Get list of available assets (fields)

        Returns:
            List of Asset objects
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return []

        try:
            # Get all cases and extract unique assets
            cases = self._explorer.cases
            assets_dict = {}

            for case in cases:
                asset_name = getattr(case, "name", "Unknown")
                asset_id = getattr(case, "uuid", asset_name)

                if asset_id not in assets_dict:
                    assets_dict[asset_id] = Asset(
                        asset_id=asset_id, kind="field", name=asset_name
                    )

            return list(assets_dict.values())
        except Exception as e:
            logger.error(f"Failed to get assets: {e}")
            return []

    def get_cases(self, field_name: str) -> List[Case]:
        """
        Get cases for a specific field

        Args:
            field_name: Name of the field/asset

        Returns:
            List of Case objects
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return []

        try:
            cases = self._explorer.cases
            result = []

            for case in cases:
                case_name = getattr(case, "name", "Unknown")
                case_id = getattr(case, "uuid", "")

                # Filter by field name if specified
                if field_name and case_name != field_name:
                    continue

                result.append(
                    Case(
                        case_id=case_id,
                        kind="case",
                        name=case_name,
                        asset_id=field_name,
                    )
                )

            return result
        except Exception as e:
            logger.error(f"Failed to get cases for field {field_name}: {e}")
            return []

    def get_ensembles(self, case_id: str) -> List[Ensemble]:
        """
        Get ensembles for a case

        Args:
            case_id: Case identifier

        Returns:
            List of Ensemble objects
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return []

        try:
            # Get case
            case = self._explorer.get_case_by_uuid(case_id)
            if not case:
                logger.warning(f"Case not found: {case_id}")
                return []

            # Get iterations (ensembles)
            iterations = case.iterations
            result = []

            for iteration in iterations:
                ensemble_name = iteration.get("name", "Unknown")
                result.append(Ensemble(ensemble_name=ensemble_name, case_id=case_id))

            return result
        except Exception as e:
            logger.error(f"Failed to get ensembles for case {case_id}: {e}")
            return []

    def get_vector_names(self, case_id: str, ensemble_name: str) -> List[VectorInfo]:
        """
        Get available vector names for a case/ensemble

        Args:
            case_id: Case identifier
            ensemble_name: Ensemble name

        Returns:
            List of VectorInfo objects
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return []

        try:
            # Get case and iteration
            case = self._explorer.get_case_by_uuid(case_id)
            if not case:
                logger.warning(f"Case not found: {case_id}")
                return []

            iteration = case.get_iteration_by_name(ensemble_name)
            if not iteration:
                logger.warning(f"Ensemble not found: {ensemble_name}")
                return []

            # Get summary vectors
            summaries = iteration.get_summaries()
            vector_names = set()

            for summary in summaries:
                columns = summary.columns
                for col in columns:
                    if col != "DATE" and col != "TIME":
                        vector_names.add(col)

            return [VectorInfo(name=name) for name in sorted(vector_names)]
        except Exception as e:
            logger.error(
                f"Failed to get vectors for case {case_id}, ensemble {ensemble_name}: {e}"
            )
            return []

    def get_realizations(
        self, case_id: str, ensemble_name: str
    ) -> List[RealizationInfo]:
        """
        Get realization IDs for a case/ensemble

        Args:
            case_id: Case identifier
            ensemble_name: Ensemble name

        Returns:
            List of RealizationInfo objects
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return []

        try:
            # Get case and iteration
            case = self._explorer.get_case_by_uuid(case_id)
            if not case:
                logger.warning(f"Case not found: {case_id}")
                return []

            iteration = case.get_iteration_by_name(ensemble_name)
            if not iteration:
                logger.warning(f"Ensemble not found: {ensemble_name}")
                return []

            # Get realizations
            realizations = iteration.realizations
            return [
                RealizationInfo(realization_id=real_id) for real_id in realizations
            ]
        except Exception as e:
            logger.error(
                f"Failed to get realizations for case {case_id}, ensemble {ensemble_name}: {e}"
            )
            return []

    def get_summary_data(
        self, case_id: str, ensemble_name: str, vector_name: str
    ) -> bytes:
        """
        Get summary data as parquet bytes

        Args:
            case_id: Case identifier
            ensemble_name: Ensemble name
            vector_name: Vector name

        Returns:
            Parquet data as bytes
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return b""

        try:
            # Get case and iteration
            case = self._explorer.get_case_by_uuid(case_id)
            if not case:
                logger.warning(f"Case not found: {case_id}")
                return b""

            iteration = case.get_iteration_by_name(ensemble_name)
            if not iteration:
                logger.warning(f"Ensemble not found: {ensemble_name}")
                return b""

            # Get summary data
            summaries = iteration.get_summaries()
            if not summaries:
                logger.warning("No summaries found")
                return b""

            # Combine data from all realizations
            all_data = []
            for summary in summaries:
                df = summary.to_pandas()
                if vector_name in df.columns:
                    # Add realization column
                    df["REAL"] = summary.realization
                    # Select relevant columns
                    df = df[["DATE", "REAL", vector_name]]
                    all_data.append(df)

            if not all_data:
                logger.warning(f"No data found for vector {vector_name}")
                return b""

            # Combine all dataframes
            combined_df = pd.concat(all_data, ignore_index=True)

            # Convert to parquet
            buffer = io.BytesIO()
            combined_df.to_parquet(buffer, index=False)
            return buffer.getvalue()
        except Exception as e:
            logger.error(
                f"Failed to get summary data for {case_id}/{ensemble_name}/{vector_name}: {e}"
            )
            return b""

    def get_parameters(self, case_id: str, ensemble_name: str) -> bytes:
        """
        Get parameter data as parquet bytes

        Args:
            case_id: Case identifier
            ensemble_name: Ensemble name

        Returns:
            Parquet data as bytes
        """
        if not self._connected or self._explorer is None:
            logger.warning("Not connected to Sumo")
            return b""

        try:
            # Get case and iteration
            case = self._explorer.get_case_by_uuid(case_id)
            if not case:
                logger.warning(f"Case not found: {case_id}")
                return b""

            iteration = case.get_iteration_by_name(ensemble_name)
            if not iteration:
                logger.warning(f"Ensemble not found: {ensemble_name}")
                return b""

            # Get parameters
            parameters = iteration.get_parameters()
            if not parameters:
                logger.warning("No parameters found")
                return b""

            # Convert to pandas DataFrame
            df = parameters.to_pandas()

            # Convert to parquet
            buffer = io.BytesIO()
            df.to_parquet(buffer, index=False)
            return buffer.getvalue()
        except Exception as e:
            logger.error(
                f"Failed to get parameters for {case_id}/{ensemble_name}: {e}"
            )
            return b""

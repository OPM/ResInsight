"""
Pydantic models for Sumo Explorer API responses
"""

from pydantic import BaseModel, Field
from typing import Optional


class Asset(BaseModel):
    """Represents a Sumo asset (field)"""

    asset_id: str = Field(..., description="Unique identifier for the asset")
    kind: str = Field(..., description="Type of asset")
    name: str = Field(..., description="Display name of the asset")


class Case(BaseModel):
    """Represents a Sumo case"""

    case_id: str = Field(..., description="Unique identifier for the case")
    kind: str = Field(..., description="Type of case")
    name: str = Field(..., description="Display name of the case")
    asset_id: Optional[str] = Field(None, description="Parent asset ID")


class Ensemble(BaseModel):
    """Represents a Sumo ensemble"""

    ensemble_name: str = Field(..., description="Name of the ensemble")
    case_id: str = Field(..., description="Parent case ID")


class VectorInfo(BaseModel):
    """Information about an available summary vector"""

    name: str = Field(..., description="Vector name (e.g., WOPR:OP1)")


class RealizationInfo(BaseModel):
    """Information about a realization"""

    realization_id: int = Field(..., description="Realization number")


class SummaryDataResponse(BaseModel):
    """Response containing summary data as Base64-encoded parquet"""

    case_id: str = Field(..., description="Case identifier")
    ensemble_name: str = Field(..., description="Ensemble name")
    vector_name: Optional[str] = Field(None, description="Vector name (if single vector)")
    data_base64: str = Field(..., description="Base64-encoded parquet data")
    row_count: int = Field(..., description="Number of rows in the data")


class ParametersResponse(BaseModel):
    """Response containing parameter data as Base64-encoded parquet"""

    case_id: str = Field(..., description="Case identifier")
    ensemble_name: str = Field(..., description="Ensemble name")
    data_base64: str = Field(..., description="Base64-encoded parquet data")
    row_count: int = Field(..., description="Number of rows in the data")


class HealthResponse(BaseModel):
    """Health check response"""

    status: str = Field(..., description="Server status")
    version: str = Field(..., description="Server version")


class StatusResponse(BaseModel):
    """Sumo connection status response"""

    connected: bool = Field(..., description="Whether connected to Sumo")
    environment: Optional[str] = Field(None, description="Sumo environment (e.g., 'prod')")
    error: Optional[str] = Field(None, description="Error message if not connected")

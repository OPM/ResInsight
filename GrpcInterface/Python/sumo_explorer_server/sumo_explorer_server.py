"""
Sumo Explorer FastAPI Server

Provides REST API wrapper around Sumo Explorer for ResInsight C++ connector.
"""

import base64
import logging
import argparse
from contextlib import asynccontextmanager
from typing import List, Optional

from fastapi import FastAPI, HTTPException, Query
from fastapi.responses import JSONResponse
import uvicorn

from .models import (
    Asset,
    Case,
    Ensemble,
    VectorInfo,
    RealizationInfo,
    SummaryDataResponse,
    ParametersResponse,
    HealthResponse,
    StatusResponse,
)
from .sumo_client import SumoClientWrapper

# Configure logging
logging.basicConfig(
    level=logging.INFO, format="%(asctime)s - %(name)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

# Global Sumo client
sumo_client: Optional[SumoClientWrapper] = None


@asynccontextmanager
async def lifespan(app: FastAPI):
    """FastAPI lifespan context manager"""
    global sumo_client

    # Startup
    logger.info("Starting Sumo Explorer server...")
    sumo_client = SumoClientWrapper(environment="prod")
    if sumo_client.is_connected:
        logger.info("Successfully connected to Sumo")
    else:
        logger.error(f"Failed to connect to Sumo: {sumo_client.error}")

    yield

    # Shutdown
    logger.info("Shutting down Sumo Explorer server...")
    sumo_client = None


app = FastAPI(
    title="Sumo Explorer API",
    description="REST API wrapper for Sumo Explorer",
    version="1.0.0",
    lifespan=lifespan,
)


@app.get("/health", response_model=HealthResponse)
async def health_check():
    """Health check endpoint"""
    return HealthResponse(status="healthy", version="1.0.0")


@app.get("/status", response_model=StatusResponse)
async def status():
    """Get Sumo connection status"""
    if sumo_client is None:
        return StatusResponse(
            connected=False, environment=None, error="Sumo client not initialized"
        )

    return StatusResponse(
        connected=sumo_client.is_connected,
        environment=sumo_client.environment if sumo_client.is_connected else None,
        error=sumo_client.error,
    )


@app.get("/assets", response_model=List[Asset])
async def get_assets():
    """Get list of available assets (fields)"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        assets = sumo_client.get_assets()
        return assets
    except Exception as e:
        logger.error(f"Failed to get assets: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/cases/{field_name}", response_model=List[Case])
async def get_cases(field_name: str):
    """Get cases for a specific field"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        cases = sumo_client.get_cases(field_name)
        return cases
    except Exception as e:
        logger.error(f"Failed to get cases for field {field_name}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/ensembles/{case_id}", response_model=List[Ensemble])
async def get_ensembles(case_id: str):
    """Get ensembles for a case"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        ensembles = sumo_client.get_ensembles(case_id)
        return ensembles
    except Exception as e:
        logger.error(f"Failed to get ensembles for case {case_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/summary/vectors", response_model=List[VectorInfo])
async def get_vector_names(
    case_id: str = Query(..., description="Case ID"),
    ensemble: str = Query(..., description="Ensemble name"),
):
    """Get available vector names for a case/ensemble"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        vectors = sumo_client.get_vector_names(case_id, ensemble)
        return vectors
    except Exception as e:
        logger.error(f"Failed to get vectors for {case_id}/{ensemble}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/summary/realizations", response_model=List[RealizationInfo])
async def get_realizations(
    case_id: str = Query(..., description="Case ID"),
    ensemble: str = Query(..., description="Ensemble name"),
):
    """Get realization IDs for a case/ensemble"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        realizations = sumo_client.get_realizations(case_id, ensemble)
        return realizations
    except Exception as e:
        logger.error(f"Failed to get realizations for {case_id}/{ensemble}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/summary/data", response_model=SummaryDataResponse)
async def get_summary_data(
    case_id: str = Query(..., description="Case ID"),
    ensemble: str = Query(..., description="Ensemble name"),
    vector: str = Query(..., description="Vector name"),
):
    """Get summary data as Base64-encoded parquet"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        # Get parquet data
        parquet_bytes = sumo_client.get_summary_data(case_id, ensemble, vector)

        if not parquet_bytes:
            raise HTTPException(
                status_code=404, detail=f"No data found for vector {vector}"
            )

        # Encode as Base64
        data_base64 = base64.b64encode(parquet_bytes).decode("utf-8")

        # Estimate row count (parquet metadata would be better, but this is simpler)
        row_count = len(parquet_bytes) // 100  # Rough estimate

        return SummaryDataResponse(
            case_id=case_id,
            ensemble_name=ensemble,
            vector_name=vector,
            data_base64=data_base64,
            row_count=row_count,
        )
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get summary data for {case_id}/{ensemble}/{vector}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/summary/parameters", response_model=ParametersResponse)
async def get_parameters(
    case_id: str = Query(..., description="Case ID"),
    ensemble: str = Query(..., description="Ensemble name"),
):
    """Get parameter data as Base64-encoded parquet"""
    if sumo_client is None or not sumo_client.is_connected:
        raise HTTPException(status_code=503, detail="Not connected to Sumo")

    try:
        # Get parquet data
        parquet_bytes = sumo_client.get_parameters(case_id, ensemble)

        if not parquet_bytes:
            raise HTTPException(status_code=404, detail="No parameters found")

        # Encode as Base64
        data_base64 = base64.b64encode(parquet_bytes).decode("utf-8")

        # Estimate row count
        row_count = len(parquet_bytes) // 100  # Rough estimate

        return ParametersResponse(
            case_id=case_id,
            ensemble_name=ensemble,
            data_base64=data_base64,
            row_count=row_count,
        )
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get parameters for {case_id}/{ensemble}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Sumo Explorer FastAPI Server")
    parser.add_argument(
        "--port", type=int, default=54527, help="Port to listen on (default: 54527)"
    )
    parser.add_argument(
        "--host", type=str, default="127.0.0.1", help="Host to bind to (default: 127.0.0.1)"
    )
    args = parser.parse_args()

    logger.info(f"Starting server on {args.host}:{args.port}")
    uvicorn.run(app, host=args.host, port=args.port)


if __name__ == "__main__":
    main()

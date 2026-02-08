# Sumo Explorer FastAPI Server

## Overview

This server provides a REST API wrapper around the Sumo Explorer Python API, enabling ResInsight's C++ code to interact with Sumo Cloud data without implementing OAuth2 authentication in C++.

## Architecture

```
ResInsight (C++) → RiaSumoExplorerConnector → FastAPI Server → Sumo Explorer API → Sumo Cloud
```

## Installation

```bash
pip install -r requirements.txt
```

## Running the Server

```bash
python -m uvicorn sumo_explorer_server.sumo_explorer_server:app --host 127.0.0.1 --port 54527
```

Or use the launcher script:

```bash
python sumo_explorer_server.py --port 54527
```

## API Endpoints

- `GET /health` - Health check
- `GET /status` - Sumo connection status
- `GET /assets` - List available assets (fields)
- `GET /cases/{field_name}` - Get cases for a specific field
- `GET /ensembles/{case_id}` - Get ensembles for a case
- `GET /summary/vectors?case_id=X&ensemble=Y` - Get available vector names
- `GET /summary/realizations?case_id=X&ensemble=Y` - Get realization IDs
- `GET /summary/data?case_id=X&ensemble=Y&vector=Z` - Get summary data (Base64-encoded parquet)
- `GET /summary/parameters?case_id=X&ensemble=Y` - Get parameters (Base64-encoded parquet)

## Configuration

The server uses the Sumo Explorer API with default authentication. Ensure you have valid Sumo credentials configured in your environment.

## Security

- Server binds to 127.0.0.1 only (localhost)
- Not exposed to external networks
- Authentication handled by Sumo Explorer API

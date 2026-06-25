


from fastapi import APIRouter


router = APIRouter(tags=["health"])

@router.get("/alive")
def alive() -> dict[str, str]:
    """Health-check endpoint polled by ResInsight for service life cycle management."""
    return {"status": "alive"}
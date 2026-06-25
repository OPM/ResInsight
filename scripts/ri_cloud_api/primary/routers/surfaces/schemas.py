from pydantic import BaseModel


class SurfaceInfo(BaseModel):
    name: str

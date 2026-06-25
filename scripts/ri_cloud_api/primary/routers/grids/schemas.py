from pydantic import BaseModel


class GridInfo(BaseModel):
    name: str
    realizations: list[int]


class GridPropertyInfo(BaseModel):
    propertyName: str
    isoDateOrInterval: str | None = None

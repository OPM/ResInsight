from pydantic import BaseModel


class AssetInfo(BaseModel):
    name: str


class CaseInfo(BaseModel):
    id: str
    name: str
    asset: str | None = None
    field: str | None = None
    status: str | None = None
    user: str | None = None
    # TODO: add ensemble info here, to avoid an extra query per case


class EnsembleInfo(BaseModel):
    name: str
    # TODO: include realization ids once we can get them cheaply per ensemble

"""Typed return values for the grid access layer."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional


@dataclass(frozen=True)
class GridInfo:
    """A grid name together with the realizations it is available for."""

    name: str
    realizations: list[int]


@dataclass(frozen=True)
class GridPropertyInfo:
    """Information about a grid property, including its name and data type."""

    property_name: str
    iso_date_or_interval: Optional[str] = None
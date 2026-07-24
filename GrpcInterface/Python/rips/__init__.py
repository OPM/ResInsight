import logging
import os
import sys
from typing import List

# Configure null handler to prevent "No handler found" warnings
logging.getLogger("rips").addHandler(logging.NullHandler())

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "generated"))

from . import category_mapping as category_mapping
from . import well_events as well_events
from . import well_log as well_log
from . import well_path_collection as well_path_collection
from .case import Case as Case
from .case import EclipseCase as EclipseCase
from .case import GeoMechCase as GeoMechCase
from .contour_map import (
    EclipseContourMap as EclipseContourMap,
)
from .contour_map import (
    GeoMechContourMap as GeoMechContourMap,
)
from .exception import RipsError as RipsError
from .grid import Grid as Grid
from .instance import Instance as Instance
from .plot import Plot as Plot
from .plot import PlotWindow as PlotWindow
from .project import Project as Project
from .resinsight_classes import *
from .simulation_well import SimulationWell as SimulationWell
from .surface import RegularSurface as RegularSurface
from .view import View as View
from .well_log_plot import WellLogPlot as WellLogPlot
from .well_path import WellPath as WellPath

__all__: list[str] = []
for key in class_dict():
    __all__.append(key)

# Add classes not in resinsight_classes
__all__.append("Grid")
__all__.append("Instance")

# PropertyType, PropertyDataType, and PorosityModelType are auto-generated
# StrEnum classes (driven by their caf::AppEnum<T> registrations) that the
# wildcard import above pulls in but class_dict() does not list.
for _enum_name in (
    "PorosityModelType",
    "PropertyDataType",
    "PropertyType",
):
    if _enum_name not in __all__:
        __all__.append(_enum_name)

__all__.sort()

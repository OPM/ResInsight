import logging
import os
import sys
from typing import List

# Configure null handler to prevent "No handler found" warnings
logging.getLogger("rips").addHandler(logging.NullHandler())

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "generated"))

from .resinsight_classes import *  # noqa: F403, E402

from .case import Case as Case, EclipseCase as EclipseCase, GeoMechCase as GeoMechCase  # noqa: E402
from .grid import Grid as Grid  # noqa: E402
from .instance import Instance as Instance  # noqa: E402
from .view import View as View  # noqa: E402
from .project import Project as Project  # noqa: E402
from .plot import Plot as Plot, PlotWindow as PlotWindow  # noqa: E402
from .contour_map import (  # noqa: E402
    EclipseContourMap as EclipseContourMap,
    GeoMechContourMap as GeoMechContourMap,
)
from .well_log_plot import WellLogPlot as WellLogPlot  # noqa: E402
from .well_path import WellPath as WellPath  # noqa: E402
from . import well_path_collection as well_path_collection  # noqa: F401, E402
from .simulation_well import SimulationWell as SimulationWell  # noqa: E402
from .exception import RipsError as RipsError  # noqa: E402
from .surface import RegularSurface as RegularSurface  # noqa: E402
from . import well_events as well_events  # noqa: F401, E402
from . import category_mapping as category_mapping  # noqa: F401, E402

__all__: List[str] = []
for key in class_dict():  # noqa: F405
    __all__.append(key)

# Add classes not in resinsight_classes
__all__.append("Grid")
__all__.append("Instance")

__all__.sort()

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "generated"))

from .resinsight_classes import *

from .case import Case as Case, EclipseCase as EclipseCase, GeoMechCase as GeoMechCase
from .grid import Grid as Grid
from .instance import Instance as Instance
from .view import View as View
from .project import Project as Project
from .plot import Plot as Plot, PlotWindow as PlotWindow
from .contour_map import (
    EclipseContourMap as EclipseContourMap,
    GeoMechContourMap as GeoMechContourMap,
)
from .well_log_plot import WellLogPlot as WellLogPlot
from .well_path import WellPath as WellPath
from . import well_path_collection
from .simulation_well import SimulationWell as SimulationWell
from .exception import RipsError as RipsError
from .surface import RegularSurface as RegularSurface

from typing import List

__all__: List[str] = []
for key in class_dict():
    __all__.append(key)

# Add classes not in resinsight_classes
__all__.append("Grid")
__all__.append("Instance")

__all__.sort()

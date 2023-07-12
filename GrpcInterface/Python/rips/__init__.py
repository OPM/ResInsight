name = "rips"

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "generated"))

from .resinsight_classes import *

from .case import Case, EclipseCase, GeoMechCase
from .grid import Grid
from .instance import Instance
from .view import View
from .project import Project
from .plot import Plot, PlotWindow
from .contour_map import EclipseContourMap, GeoMechContourMap
from .well_log_plot import WellLogPlot
from .simulation_well import SimulationWell

from typing import List

__all__: List[str] = []
for key in class_dict():
    __all__.append(key)

# Add classes not in resinsight_classes
__all__.append("Grid")
__all__.append("Instance")

__all__.sort()

name = "rips"

import os
import sys
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'generated'))

from rips.case import Case, EclipseCase, GeoMechCase
from rips.grid import Grid
from rips.instance import Instance
from rips.pdmobject import PdmObject
from rips.view import View
from rips.project import Project
from rips.plot import Plot, PlotWindow
from rips.contour_map import EclipseContourMap, GeoMechContourMap
from rips.well_log_plot import WellLogPlot
from rips.simulation_well import SimulationWell

from rips.generated.pdm_objects import *

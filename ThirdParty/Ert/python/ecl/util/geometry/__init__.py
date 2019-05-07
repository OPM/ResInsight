#  Copyright (C) 2011  Equinor ASA, Norway. 
#   
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 
"""
Simple package for working with 2D geometry.

"""
import ecl
from cwrap import Prototype

from .geo_pointset import GeoPointset
from .geo_region import GeoRegion
from .cpolyline import CPolyline
from .cpolyline_collection import CPolylineCollection
from .polyline import Polyline
from .xyz_io import XYZIo
from .geometry_tools import GeometryTools
from .surface import Surface

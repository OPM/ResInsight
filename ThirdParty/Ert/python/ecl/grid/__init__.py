#  Copyright (C) 2018  Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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
  ecl_grid/EclGrid: This will load an ECLIPSE GRID or EGRID file, and
     can then subsequently be used for queries about the grid.

  ecl_region/EclRegion: Convenience class to support selecting cells
     in a grid based on a wide range of criteria. Can be used as a
     mask in operations on EclKW instances.

  ecl_grid_generator/EclGridGenerator: This can be used to generate various
    grids.
"""

import ecl.util.util
import ecl.util.geometry

from .cell import Cell
from .ecl_grid import EclGrid
from .ecl_region import EclRegion
from .ecl_grid_generator import EclGridGenerator

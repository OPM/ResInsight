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
  ecl_rft/[EclRFTFile , EclRFT , EclRFTCell]: Loads an ECLIPSE RFT/PLT
     file, and can afterwords be used to support various queries.
"""

from .well_trajectory import WellTrajectory
from .ecl_rft_cell import EclPLTCell, EclRFTCell
from .ecl_rft import EclRFT, EclRFTFile


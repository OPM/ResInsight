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
  ecl_grav/EclGrav: Class used to simplify evaluation of ECLIPSE
     modelling time-lapse gravitational surveys.

  ecl_subsidence/EclSubsidence: Small class used to evaluate simulated
     subsidence from ECLIPSE simulations; analogous to the EcLGrav
     functionality.
"""

from .ecl_subsidence import EclSubsidence
from .ecl_grav_calc import phase_deltag, deltag
from .ecl_grav import EclGrav

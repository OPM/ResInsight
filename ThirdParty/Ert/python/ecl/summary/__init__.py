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
  ecl_sum/EclSum: This will load summary results from an ECLIPSE run;
     both data file(s) and the SMSPEC file. The EclSum object can be
     used as basis for queries on summary vectors.
"""


import ecl.util.util
import ecl.util.geometry

from .ecl_sum_var_type import EclSumVarType
from .ecl_sum_tstep import EclSumTStep
from .ecl_sum import EclSum #, EclSumVector, EclSumNode, EclSMSPECNode
from .ecl_sum_keyword_vector import EclSumKeyWordVector
from .ecl_sum_node import EclSumNode
from .ecl_sum_vector import EclSumVector
from .ecl_npv import EclNPV , NPVPriceVector
from .ecl_cmp import EclCmp


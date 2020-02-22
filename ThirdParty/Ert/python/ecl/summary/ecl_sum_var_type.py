#  Copyright (C) 2016  Equinor ASA, Norway.
#
#  The file 'ecl_sum_var_type.py' is part of ERT - Ensemble based Reservoir Tool.
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

from cwrap import BaseCEnum



class EclSumVarType(BaseCEnum):
    TYPE_NAME = "ecl_sum_var_type"
    ECL_SMSPEC_INVALID_VAR            = None
    ECL_SMSPEC_FIELD_VAR              = None
    ECL_SMSPEC_REGION_VAR             = None
    ECL_SMSPEC_GROUP_VAR              = None
    ECL_SMSPEC_WELL_VAR               = None
    ECL_SMSPEC_SEGMENT_VAR            = None
    ECL_SMSPEC_BLOCK_VAR              = None
    ECL_SMSPEC_AQUIFER_VAR            = None
    ECL_SMSPEC_COMPLETION_VAR         = None
    ECL_SMSPEC_NETWORK_VAR            = None
    ECL_SMSPEC_REGION_2_REGION_VAR    = None
    ECL_SMSPEC_LOCAL_BLOCK_VAR        = None
    ECL_SMSPEC_LOCAL_COMPLETION_VAR   = None
    ECL_SMSPEC_LOCAL_WELL_VAR         = None
    ECL_SMSPEC_MISC_VAR               = None


EclSumVarType.addEnum("ECL_SMSPEC_INVALID_VAR", 0)
EclSumVarType.addEnum("ECL_SMSPEC_FIELD_VAR", 1)
EclSumVarType.addEnum("ECL_SMSPEC_REGION_VAR", 2)
EclSumVarType.addEnum("ECL_SMSPEC_GROUP_VAR", 3)
EclSumVarType.addEnum("ECL_SMSPEC_WELL_VAR", 4)
EclSumVarType.addEnum("ECL_SMSPEC_SEGMENT_VAR", 5)
EclSumVarType.addEnum("ECL_SMSPEC_BLOCK_VAR", 6)
EclSumVarType.addEnum("ECL_SMSPEC_AQUIFER_VAR", 7)
EclSumVarType.addEnum("ECL_SMSPEC_COMPLETION_VAR", 8)
EclSumVarType.addEnum("ECL_SMSPEC_NETWORK_VAR", 9)
EclSumVarType.addEnum("ECL_SMSPEC_REGION_2_REGION_VAR", 10)
EclSumVarType.addEnum("ECL_SMSPEC_LOCAL_BLOCK_VAR", 11)
EclSumVarType.addEnum("ECL_SMSPEC_LOCAL_COMPLETION_VAR", 12)
EclSumVarType.addEnum("ECL_SMSPEC_LOCAL_WELL_VAR", 13)
EclSumVarType.addEnum("ECL_SMSPEC_MISC_VAR", 14)
    


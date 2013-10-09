#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'content_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.cwrap import BaseCEnum
from ert.enkf import ENKF_LIB


class EnkfStateType(BaseCEnum):
    UNDEFINED = None
    FORECAST = None
    ANALYZED = None
    BOTH = None

    INITIALIZATION_TYPES = None


# EnkfStateType.addEnum("UNDEFINED", 0)
# EnkfStateType.addEnum("FORECAST", 2)
# EnkfStateType.addEnum("ANALYZED", 4)
# EnkfStateType.addEnum("BOTH", 6)
EnkfStateType.populateEnum(ENKF_LIB, "enkf_state_enum_iget")
EnkfStateType.registerEnum(ENKF_LIB, "enkf_state_type_enum")

EnkfStateType.INITIALIZATION_TYPES = [EnkfStateType.ANALYZED, EnkfStateType.FORECAST]




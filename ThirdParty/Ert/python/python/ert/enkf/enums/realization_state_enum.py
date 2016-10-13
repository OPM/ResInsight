#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'ert_impl_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import ENKF_LIB


class RealizationStateEnum(BaseCEnum):
    STATE_UNDEFINED      = None
    STATE_INITIALIZED    = None
    STATE_HAS_DATA       = None
    STATE_LOAD_FAILURE   = None
    STATE_PARENT_FAILURE = None

RealizationStateEnum.addEnum("STATE_UNDEFINED", 1)
RealizationStateEnum.addEnum("STATE_INITIALIZED", 2)
RealizationStateEnum.addEnum("STATE_HAS_DATA", 4)
RealizationStateEnum.addEnum("STATE_LOAD_FAILURE", 8)
RealizationStateEnum.addEnum("STATE_PARENT_FAILURE", 16)
RealizationStateEnum.registerEnum(ENKF_LIB, "realisation_state_enum")




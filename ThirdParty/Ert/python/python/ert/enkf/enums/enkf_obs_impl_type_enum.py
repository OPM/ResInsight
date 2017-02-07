#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'enkf_obs_impl_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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

class EnkfObservationImplementationType(BaseCEnum):
    TYPE_NAME = "enkf_obs_impl_type"
    GEN_OBS = None
    SUMMARY_OBS = None
    BLOCK_OBS = None

EnkfObservationImplementationType.addEnum("GEN_OBS", 1)
EnkfObservationImplementationType.addEnum("SUMMARY_OBS", 2)
EnkfObservationImplementationType.addEnum("BLOCK_OBS", 3)

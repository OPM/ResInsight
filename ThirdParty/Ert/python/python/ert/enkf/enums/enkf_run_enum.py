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
from cwrap import BaseCEnum
from ert.enkf import ENKF_LIB


class EnkfRunType(BaseCEnum):
    ENKF_ASSIMILATION = None
    ENSEMBLE_EXPERIMENT = None
    SMOOTHER_UPDATED = None
    INIT_ONLY = None
    

EnkfRunType.addEnum("ENKF_ASSIMILATION" , 1)
EnkfRunType.addEnum("ENSEMBLE_EXPERIMENT" , 2)
EnkfRunType.addEnum("SMOOTHER_UPDATE" , 4)
EnkfRunType.addEnum("INIT_ONLY" , 8)

EnkfRunType.registerEnum( ENKF_LIB , "enkf_run_mode_enum")


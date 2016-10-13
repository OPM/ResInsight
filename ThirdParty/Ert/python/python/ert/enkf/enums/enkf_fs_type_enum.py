#  Copyright (C) 2014  Statoil ASA, Norway.
#
#  The file 'enkf_fs_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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


class EnKFFSType(BaseCEnum):
    INVALID_DRIVER_ID = None
    PLAIN_DRIVER_ID = None
    BLOCK_FS_DRIVER_ID = None


EnKFFSType.addEnum("INVALID_DRIVER_ID", 0)
EnKFFSType.addEnum("PLAIN_DRIVER_ID", 1005)
EnKFFSType.addEnum("BLOCK_FS_DRIVER_ID", 3001)
EnKFFSType.registerEnum(ENKF_LIB, "enkf_fs_type_enum")




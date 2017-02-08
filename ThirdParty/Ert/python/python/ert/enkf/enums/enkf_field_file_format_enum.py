#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'enkf_field_file_format_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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

class EnkfFieldFileFormatEnum(BaseCEnum):
    TYPE_NAME = "enkf_field_file_format_enum"
    UNDEFINED_FORMAT         = None
    RMS_ROFF_FILE            = None
    ECL_KW_FILE              = None
    ECL_KW_FILE_ACTIVE_CELLS = None
    ECL_KW_FILE_ALL_CELLS    = None
    ECL_GRDECL_FILE          = None
    ECL_FILE                 = None
    FILE_FORMAT_NULL         = None

EnkfFieldFileFormatEnum.addEnum("UNDEFINED_FORMAT", 0)
EnkfFieldFileFormatEnum.addEnum("RMS_ROFF_FILE", 1)
EnkfFieldFileFormatEnum.addEnum("ECL_KW_FILE", 2)
EnkfFieldFileFormatEnum.addEnum("ECL_KW_FILE_ACTIVE_CELLS", 3)
EnkfFieldFileFormatEnum.addEnum("ECL_KW_FILE_ALL_CELLS", 4)
EnkfFieldFileFormatEnum.addEnum("ECL_GRDECL_FILE", 5)
EnkfFieldFileFormatEnum.addEnum("ECL_FILE", 6)
EnkfFieldFileFormatEnum.addEnum("FILE_FORMAT_NULL", 7)

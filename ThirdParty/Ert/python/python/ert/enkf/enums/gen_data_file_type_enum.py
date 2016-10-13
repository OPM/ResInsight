#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'gen_data_file_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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


class GenDataFileType(BaseCEnum):
    GEN_DATA_UNDEFINED      = None
    ASCII                   = None       # The file is ASCII file with a vector of numbers formatted with "%g"
    ASCII_TEMPLATE          = None       # The data is inserted into a user defined template file.
    BINARY_DOUBLE           = None       #  The data is in a binary file with doubles.
    BINARY_FLOAT            = None       # The data is in a binary file with floats.


GenDataFileType.addEnum("GEN_DATA_UNDEFINED", 0)
GenDataFileType.addEnum("ASCII", 1)
GenDataFileType.addEnum("ASCII_TEMPLATE", 2)
GenDataFileType.addEnum("BINARY_DOUBLE", 3)
GenDataFileType.addEnum("BINARY_FLOAT", 4)
GenDataFileType.registerEnum(ENKF_LIB, "gen_data_file_format_type")






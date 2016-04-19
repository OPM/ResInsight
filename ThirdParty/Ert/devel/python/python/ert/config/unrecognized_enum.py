#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'unrecognized_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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

class UnrecognizedEnum(BaseCEnum):
    TYPE_NAME = "config_unrecognized_enum"
    CONFIG_UNRECOGNIZED_IGNORE = None
    CONFIG_UNRECOGNIZED_WARN = None
    CONFIG_UNRECOGNIZED_ERROR = None

UnrecognizedEnum.addEnum("CONFIG_UNRECOGNIZED_IGNORE", 0)
UnrecognizedEnum.addEnum("CONFIG_UNRECOGNIZED_WARN", 1)
UnrecognizedEnum.addEnum("CONFIG_UNRECOGNIZED_ERROR", 2)

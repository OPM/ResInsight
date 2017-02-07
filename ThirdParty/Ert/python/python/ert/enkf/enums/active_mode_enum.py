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

class ActiveMode(BaseCEnum):
    TYPE_NAME = "active_mode_enum"
    ALL_ACTIVE = None
    INACTIVE = None
    PARTLY_ACTIVE = None


ActiveMode.addEnum("ALL_ACTIVE", 1)
ActiveMode.addEnum("INACTIVE", 2)
ActiveMode.addEnum("PARTLY_ACTIVE", 3)

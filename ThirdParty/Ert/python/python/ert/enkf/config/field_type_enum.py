#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

class FieldTypeEnum(BaseCEnum):
    TYPE_NAME = "field_type_enum"
    ECLIPSE_RESTART    = None
    ECLIPSE_PARAMETER  = None
    GENERAL            = None
    UNKNOWN_FIELD_TYPE = None

FieldTypeEnum.addEnum('ECLIPSE_RESTART',    1)
FieldTypeEnum.addEnum('ECLIPSE_PARAMETER',  2)
FieldTypeEnum.addEnum('GENERAL',            3)
FieldTypeEnum.addEnum('UNKNOWN_FIELD_TYPE', 4)

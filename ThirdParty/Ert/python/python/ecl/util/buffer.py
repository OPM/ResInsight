#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'buffer.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from cwrap import BaseCClass
from ecl.util import UtilPrototype


class Buffer(BaseCClass):
    _alloc = UtilPrototype("void* buffer_alloc(int)" , bind = False)
    _free = UtilPrototype("void buffer_free(buffer)")

    def __init__(self, size):
        super(Buffer, self).__init__(self._alloc(size))

    def free(self):
        self._free()

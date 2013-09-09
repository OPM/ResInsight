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

from ert.cwrap import BaseCClass, CWrapper
from ert.util import UTIL_LIB


class Buffer(BaseCClass):
    def __init__(self, size):
        super(Buffer, self).__init__(Buffer.cNamespace().alloc(size))

    def free(self):
        Buffer.cNamespace().free(self)

##################################################################

cwrapper = CWrapper(UTIL_LIB)
cwrapper.registerType("buffer", Buffer)
cwrapper.registerType("buffer_obj", Buffer.createPythonObject)
cwrapper.registerType("buffer_ref", Buffer.createCReference)


Buffer.cNamespace().free = cwrapper.prototype("void buffer_free(buffer)")
Buffer.cNamespace().alloc = cwrapper.prototype("c_void_p buffer_alloc(int)")

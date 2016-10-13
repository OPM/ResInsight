#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'cstring.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import ctypes

from ert.util import UtilPrototype

_free = UtilPrototype("void free(void*)")


def cStringObject(c_ptr):
    """The cStringObject function is a convenience function which creates a
    Python string copy, and discards the underlying C allocated storage
    for strings created with *alloc() functions in C.

    This function should not be invoked directly, only indirectly
    through the prototyping of the symbol 'cstring_obj'.

    """
    if c_ptr is not None:
        python_string = ctypes.c_char_p(c_ptr).value
        _free(c_ptr)
        return python_string
    else:
        return None


UtilPrototype.registerType("cstring_obj", cStringObject)

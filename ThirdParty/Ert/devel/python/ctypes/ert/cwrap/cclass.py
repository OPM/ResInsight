#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'cclass.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class CClass(object):
    """
    Base class used by all the classes wrapping a C 'class'.

    All Python classes which wrap a C 'class', like e.g. the
    ecl_kw_type structure, need a from_param() classmethod which is
    used to 'translate' between the python object and the pointer to
    corresponding underlying C structure. The CClass class contains an
    implementation of such a from_param() method; all Python classes
    which wrap a C 'class' should inherit from this class.

    Observe that this implementation makes a nasty assumption that the
    actual implementation, i.e. EclKW should store the underlying C
    pointer in the attribute 'c_ptr' - otherwise the from_param()
    method will blow up at first use.
    """
    c_ptr = None

    @classmethod
    def from_param( cls , obj ):
        if obj is None:
            return ctypes.c_void_p()
        else:
            return ctypes.c_void_p( obj.c_ptr )

#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'field.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
import  libenkf
class FieldObs(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    def ijk_get_double(self , i,j,k):
        return cfunc.ijk_get_double( self ,i,j,k )



##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "field" , FieldObs )

# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("field")


cfunc.free                = cwrapper.prototype("void field_free( field )")
cfunc.ijk_get_double      = cwrapper.prototype("double field_ijk_get_double(field, int, int, int)")

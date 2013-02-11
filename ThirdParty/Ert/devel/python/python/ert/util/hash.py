#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'hash.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import sys
import ctypes
import libutil
from   ert.cwrap.cwrap       import *


class Hash:

    def __init__( self ):
        self.c_ptr = cfunc.hash_alloc()
        
    def __del__( self ):
        cfunc.hash_del( self )
        
    def __getitem__(self , key):
        pass


CWrapper.registerType( "hash" , Hash )
cwrapper         = CWrapper( libutil.lib )
cfunc            = CWrapperNameSpace("hash")

cfunc.hash_alloc = cwrapper.prototype("long hash_alloc( )")
cfunc.hash_free  = cwrapper.prototype("void hash_free( hash )")
cfunc.hash_get   = cwrapper.prototype("long hash_get(hash , char*)")

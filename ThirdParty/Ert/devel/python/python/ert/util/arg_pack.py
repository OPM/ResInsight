#  Copyright (C) 2015  Statoil ASA, Norway. 
#   
#  The file 'arg_pack.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert.util import UTIL_LIB
from ert.cwrap import CWrapper, BaseCClass


class ArgPack(BaseCClass):

    def __init__(self , *args):
        c_ptr = ArgPack.cNamespace().alloc()
        super(ArgPack, self).__init__(c_ptr)
        self.child_list = []
        for arg in args:
            self.append( arg )

            
        
    def append(self , data):
        if isinstance(data , int):
            ArgPack.cNamespace().append_int( self , data )
        elif isinstance(data , float):
            ArgPack.cNamespace().append_double( self , data )
        elif isinstance(data , BaseCClass):
            ArgPack.cNamespace().append_ptr( self , BaseCClass.from_param( data ) )
            self.child_list.append( data )
        else:
            raise TypeError("Can only add int/double/basecclass")

        
    def __len__(self):
        return ArgPack.cNamespace().size(self)

    

    def free(self):
        ArgPack.cNamespace().free(self)



CWrapper.registerObjectType("arg_pack", ArgPack)

cwrapper = CWrapper(UTIL_LIB)

ArgPack.cNamespace().alloc          = cwrapper.prototype("c_void_p arg_pack_alloc( )")
ArgPack.cNamespace().free           = cwrapper.prototype("void arg_pack_free(arg_pack )")
ArgPack.cNamespace().size           = cwrapper.prototype("int arg_pack_size(arg_pack )")
ArgPack.cNamespace().append_int     = cwrapper.prototype("void arg_pack_append_int(arg_pack , int)")
ArgPack.cNamespace().append_double  = cwrapper.prototype("void arg_pack_append_double(arg_pack ,double)")
ArgPack.cNamespace().append_ptr     = cwrapper.prototype("void arg_pack_append_ptr(arg_pack , c_void_p)")

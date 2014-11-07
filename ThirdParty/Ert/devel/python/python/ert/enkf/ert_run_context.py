#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'enkf_fs.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB, TimeMap, StateMap, RunArg
from ert.enkf.enums import EnKFFSType

class ErtRunContext(BaseCClass):
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        return ErtRunContext.cNamespace().get_size( self )


    def __getitem__(self , index):
        if isinstance(index, int):
            if 0 <= index < len(self):
                run_arg = ErtRunContext.cNamespace().iget(self , index)
                run_arg.setParent( self )
                return run_arg
            else:
                raise IndexError("Index:%d invalid. Legal range: [0,%d)" % (index , len(self)))
        else:
            raise TypeError("Invalid type - expetected integer")

    def __eq__(self , other):
        if self.c_ptr == other.c_ptr:
            return True
        else:
            raise Exception("__eq__ not properly implemented")


    def iensGet(self , iens):
        c_ptr = ErtRunContext.cNamespace().iens_get(self , iens)
        if c_ptr:
            run_arg = RunArg( c_ptr , parent = self , is_reference = True )
            return run_arg
        else:
            raise ValueError("Run context does not have run argument for iens:%d" % iens)
        

    def free(self):
        ErtRunContext.cNamespace().free( self )

    



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("ert_run_context", ErtRunContext)

ErtRunContext.cNamespace().get_size = cwrapper.prototype("int ert_run_context_get_size( ert_run_context )")
ErtRunContext.cNamespace().free     = cwrapper.prototype("void ert_run_context_free( ert_run_context )")
ErtRunContext.cNamespace().iget     = cwrapper.prototype("run_arg_ref ert_run_context_iget_arg( ert_run_context , int)")
ErtRunContext.cNamespace().iens_get = cwrapper.prototype("c_void_p ert_run_context_iens_get_arg( ert_run_context , int)")

        

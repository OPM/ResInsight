#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'run_arg.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.enkf import ENKF_LIB, TimeMap, StateMap
from ert.enkf.enums import EnkfRunType, EnkfStateType



class RunArg(BaseCClass):
    def __init__(self , c_ptr , parent = None , is_reference = False ):
        super(RunArg , self).__init__( c_ptr , parent = parent , is_reference = is_reference )
        
    
    @classmethod
    def ENSEMBLE_EXPERIMENT(cls , fs , iens , runpath , iter = 0):
        c_ptr = RunArg.cNamespace().alloc_ENSEMBLE_EXPERIMENT(fs , iens , iter , runpath)
        return RunArg( c_ptr )

    def free(self):
        RunArg.cNamespace().free(self)

    def getQueueIndex(self):
        return RunArg.cNamespace().get_queue_index( self )

    def isSubmitted(self):
        return RunArg.cNamespace().is_submitted( self )



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("run_arg", RunArg)


RunArg.cNamespace().alloc_ENSEMBLE_EXPERIMENT = cwrapper.prototype("c_void_p run_arg_alloc_ENSEMBLE_EXPERIMENT(enkf_fs , int, int, char*)")
RunArg.cNamespace().free  = cwrapper.prototype("void run_arg_free(run_arg)")
RunArg.cNamespace().get_queue_index  = cwrapper.prototype("int run_arg_get_queue_index(run_arg)")
RunArg.cNamespace().is_submitted  = cwrapper.prototype("bool run_arg_is_submitted(run_arg)")

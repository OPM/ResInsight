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
from ert.enkf import ENKF_LIB


class RunArg(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly")

    @classmethod
    def createEnsembleExperimentRunArg(cls, fs, iens, runpath, iter=0):
        return RunArg.cNamespace().alloc_ENSEMBLE_EXPERIMENT(fs, iens, iter, runpath)

    def free(self):
        RunArg.cNamespace().free(self)

    def getQueueIndex(self):
        return RunArg.cNamespace().get_queue_index(self)

    def isSubmitted(self):
        return RunArg.cNamespace().is_submitted(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("run_arg", RunArg)

RunArg.cNamespace().alloc_ENSEMBLE_EXPERIMENT = cwrapper.prototype("run_arg_obj run_arg_alloc_ENSEMBLE_EXPERIMENT(enkf_fs , int, int, char*)")
RunArg.cNamespace().free = cwrapper.prototype("void run_arg_free(run_arg)")
RunArg.cNamespace().get_queue_index = cwrapper.prototype("int run_arg_get_queue_index(run_arg)")
RunArg.cNamespace().is_submitted = cwrapper.prototype("bool run_arg_is_submitted(run_arg)")

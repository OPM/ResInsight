#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'forward_model.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.cwrap import CWrapper, BaseCClass
from ert.job_queue import ExtJob, JOB_QUEUE_LIB
from ert.util import StringList


class ForwardModel(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def joblist(self):
        """ @rtype: StringList """
        return ForwardModel.cNamespace().alloc_joblist(self)

    def iget_job(self, index):
        """ @rtype: ExtJob """
        return ForwardModel.cNamespace().iget_job(self, index).setParent(self)

    def add_job(self, name):
        """ @rtype: ExtJob """
        return ForwardModel.cNamespace().add_job(self, name).setParent(self)

    def clear(self):
        ForwardModel.cNamespace().clear(self)

    def free(self):
        ForwardModel.cNamespace().free(self)

cwrapper = CWrapper(JOB_QUEUE_LIB)
cwrapper.registerType("forward_model", ForwardModel)
cwrapper.registerType("forward_model_obj", ForwardModel.createPythonObject)
cwrapper.registerType("forward_model_ref", ForwardModel.createCReference)

ForwardModel.cNamespace().free = cwrapper.prototype("void forward_model_free( forward_model )")
ForwardModel.cNamespace().clear = cwrapper.prototype("void forward_model_clear(forward_model)")
ForwardModel.cNamespace().add_job = cwrapper.prototype("ext_job_ref forward_model_add_job(forward_model, char*)")
ForwardModel.cNamespace().alloc_joblist = cwrapper.prototype("stringlist_obj forward_model_alloc_joblist(forward_model)")
ForwardModel.cNamespace().iget_job = cwrapper.prototype("ext_job_ref forward_model_iget_job( forward_model, int)")

import os
import sys
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, RunpathList


class HookWorkflow(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getWorkflow(self):
        """ @rtype: Workflow """
        return HookWorkflow.cNamespace().get_workflow(self)
    
    def getRunMode(self):
        return HookWorkflow.cNamespace().get_runmode( self )

    
cwrapper = CWrapper(ENKF_LIB)

cwrapper.registerObjectType("hook_workflow", HookWorkflow)

HookWorkflow.cNamespace().get_workflow = cwrapper.prototype("workflow_ref hook_workflow_get_workflow(hook_workflow)")
HookWorkflow.cNamespace().get_runmode  = cwrapper.prototype("hook_runtime_enum hook_workflow_get_run_mode(hook_workflow)")

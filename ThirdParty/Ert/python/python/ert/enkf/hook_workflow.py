import os
import sys
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype, RunpathList

class HookWorkflow(BaseCClass):
    TYPE_NAME = "hook_workflow"

    _get_workflow = EnkfPrototype("workflow_ref hook_workflow_get_workflow(hook_workflow)")
    _get_runmode  = EnkfPrototype("hook_runtime_enum hook_workflow_get_run_mode(hook_workflow)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getWorkflow(self):
        """ @rtype: Workflow """
        return self._get_workflow()

    def getRunMode(self):
        return self._get_runmode()

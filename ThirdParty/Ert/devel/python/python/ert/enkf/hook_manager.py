import os
import sys
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB

class HookManager(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")
    
    def __len__(self):
        """ @rtype: int """
        return HookManager.cNamespace().size(self)
    
    def __getitem__(self, index):
        """ @rtype: Hook workflow """
        assert isinstance(index, int)
        if index < len(self):
            return HookManager.cNamespace().iget_hook_workflow(self, index)
        else:
            raise IndexError("Invalid index")

    def checkRunpathListFile(self):
        """ @rtype: bool """
        runpath_list_file = HookManager.cNamespace().get_runpath_list_file(self)

        if not os.path.exists(runpath_list_file):
            sys.stderr.write("** Warning: the file: %s with a list of runpath directories was not found - hook workflow will probably fail.\n" % runpath_list_file)
    
    def getRunpathList(self):
        """ @rtype: RunpathList """
        return HookManager.cNamespace().get_runpath_list(self)
        
    def runWorkflows(self , run_time , ert_self):
        
        workflow_list = ert_self.getWorkflowList()
        for hook_workflow in self:
            
            if (hook_workflow.getRunMode() is not run_time):
                continue
            
            workflow = hook_workflow.getWorkflow()            
            workflow.run(ert_self, context=workflow_list.getContext())       
    
cwrapper = CWrapper(ENKF_LIB)

cwrapper.registerObjectType("hook_manager", HookManager)

HookManager.cNamespace().get_runpath_list_file = cwrapper.prototype("char* hook_manager_get_runpath_list_file(hook_manager)")
HookManager.cNamespace().iget_hook_workflow    = cwrapper.prototype("hook_workflow_ref hook_manager_iget_hook_workflow(hook_manager, int)")
HookManager.cNamespace().size                  = cwrapper.prototype("int hook_manager_get_size(hook_manager)")

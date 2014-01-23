from ert.config import ConfigError
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import StringList


class ErtWorkflowList(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getWorkflowNames(self):
        """ @rtype: StringList """
        return ErtWorkflowList.cNamespace().alloc_namelist(self)

    def runWorkflow(self, workflow_name):
        """ @rtype: bool """
        pointer = self.parent().from_param(self.parent())
        return ErtWorkflowList.cNamespace().run_workflow(self, workflow_name, pointer)

    def __contains__(self, workflow_name):
        assert isinstance(workflow_name, str)
        return ErtWorkflowList.cNamespace().has_workflow(self, workflow_name)

    def setVerbose(self, verbose):
        ErtWorkflowList.cNamespace().set_verbose(self, verbose)

    def getLastError(self):
        """ @rtype: ConfigError """
        return ErtWorkflowList.cNamespace().get_last_error(self).setParent(self)

    def free(self):
        ErtWorkflowList.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ert_workflow_list", ErtWorkflowList)
cwrapper.registerType("ert_workflow_list_ref", ErtWorkflowList.createCReference)
cwrapper.registerType("ert_workflow_list_obj", ErtWorkflowList.createPythonObject)

ErtWorkflowList.cNamespace().free = cwrapper.prototype("void ert_workflow_list_free(ert_workflow_list)")

ErtWorkflowList.cNamespace().alloc_namelist = cwrapper.prototype("stringlist_obj ert_workflow_list_alloc_namelist(ert_workflow_list)")

ErtWorkflowList.cNamespace().run_workflow = cwrapper.prototype("bool ert_workflow_list_run_workflow(ert_workflow_list, char*, c_void_p)")
ErtWorkflowList.cNamespace().has_workflow = cwrapper.prototype("bool ert_workflow_list_has_workflow(ert_workflow_list, char*)")
ErtWorkflowList.cNamespace().set_verbose  = cwrapper.prototype("void ert_workflow_list_set_verbose(ert_workflow_list, bool)")
ErtWorkflowList.cNamespace().get_last_error  = cwrapper.prototype("config_error_ref ert_workflow_list_get_last_error(ert_workflow_list)")
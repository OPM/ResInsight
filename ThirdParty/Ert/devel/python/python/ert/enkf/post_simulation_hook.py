import os
import sys
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB, RunpathList
from ert.job_queue import Workflow

class PostSimulationHook(BaseCClass):

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def hasWorkflow(self):
        """ @rtype: bool """
        return PostSimulationHook.cNamespace().has_workflow(self)

    def getWorkflow(self):
        """ @rtype: Workflow """
        return PostSimulationHook.cNamespace().get_workflow(self)

    def checkRunpathListFile(self):
        """ @rtype: bool """
        runpath_list_file = PostSimulationHook.cNamespace().get_runpath_list_file(self)

        if not os.path.exists(runpath_list_file):
            sys.stderr.write("** Warning: the file: %s with a list of runpath directories was not found - QC workflow wil probably fail.\n" % runpath_list_file)

    def getRunpathList(self):
        """ @rtype: RunpathList """
        return PostSimulationHook.cNamespace().get_runpath_list(self)

cwrapper = CWrapper(ENKF_LIB)

cwrapper.registerObjectType("qc_module", PostSimulationHook)

PostSimulationHook.cNamespace().has_workflow = cwrapper.prototype("bool qc_module_has_workflow(qc_module)")
PostSimulationHook.cNamespace().get_workflow = cwrapper.prototype("workflow_ref qc_module_get_workflow(qc_module)")
PostSimulationHook.cNamespace().get_runpath_list_file = cwrapper.prototype("char* qc_module_get_runpath_list_file(qc_module)")
PostSimulationHook.cNamespace().get_runpath_list = cwrapper.prototype("runpath_list_ref qc_module_get_runpath_list(qc_module)")


from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import StringList, SubstitutionList
from ert.job_queue import Workflow, WorkflowJob


class ErtWorkflowList(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getWorkflowNames(self):
        """ @rtype: StringList """
        return ErtWorkflowList.cNamespace().alloc_namelist(self)

    def __contains__(self, workflow_name):
        assert isinstance(workflow_name, str)
        return ErtWorkflowList.cNamespace().has_workflow(self, workflow_name)

    def __getitem__(self, item):
        """ @rtype: Workflow """
        if not item in self:
            raise KeyError("Item '%s' is not in the list of available workflows." % item)

        return ErtWorkflowList.cNamespace().get_workflow(self, item).setParent(self)

    def getContext(self):
        """ @rtype: SubstitutionList """
        return ErtWorkflowList.cNamespace().get_context(self)

    def free(self):
        ErtWorkflowList.cNamespace().free(self)

    def addJob(self, job_name, job_path):
        """
        @type job_name: str
        @type job_path: str
        """
        ErtWorkflowList.cNamespace().add_job(self, job_name, job_path)

    def hasJob(self, job_name):
        """
         @type job_name: str
         @rtype: bool
        """
        return ErtWorkflowList.cNamespace().has_job(self, job_name)

    def getJob(self, job_name):
        """ @rtype: WorkflowJob """
        return ErtWorkflowList.cNamespace().get_job(self, job_name)

    def getJobNames(self):
        """ @rtype: StringList """
        return ErtWorkflowList.cNamespace().get_job_names(self)

    def getPluginJobs(self):
        """ @rtype: list of WorkflowJob """
        plugins = []
        for job_name in self.getJobNames():
            job = self.getJob(job_name)
            if job.isPlugin():
                plugins.append(job)
        return plugins




cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("ert_workflow_list", ErtWorkflowList)

ErtWorkflowList.cNamespace().free = cwrapper.prototype("void ert_workflow_list_free(ert_workflow_list)")
ErtWorkflowList.cNamespace().alloc_namelist = cwrapper.prototype("stringlist_obj ert_workflow_list_alloc_namelist(ert_workflow_list)")

ErtWorkflowList.cNamespace().has_workflow = cwrapper.prototype("bool ert_workflow_list_has_workflow(ert_workflow_list, char*)")
ErtWorkflowList.cNamespace().get_workflow = cwrapper.prototype("workflow_ref ert_workflow_list_get_workflow(ert_workflow_list, char*)")
ErtWorkflowList.cNamespace().get_context = cwrapper.prototype("subst_list_ref ert_workflow_list_get_context(ert_workflow_list)")

ErtWorkflowList.cNamespace().add_job = cwrapper.prototype("void ert_workflow_list_add_job(ert_workflow_list, char*, char*)")
ErtWorkflowList.cNamespace().has_job = cwrapper.prototype("bool ert_workflow_list_has_job(ert_workflow_list, char*)")
ErtWorkflowList.cNamespace().get_job = cwrapper.prototype("workflow_job_ref ert_workflow_list_get_job(ert_workflow_list, char*)")
ErtWorkflowList.cNamespace().get_job_names = cwrapper.prototype("stringlist_obj ert_workflow_list_get_job_names(ert_workflow_list)")
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import StringList, SubstitutionList
from ert.job_queue import Workflow, WorkflowJob


class ErtWorkflowList(BaseCClass):
    TYPE_NAME = "ert_workflow_list"

    _alloc_namelist = EnkfPrototype("stringlist_obj ert_workflow_list_alloc_namelist(ert_workflow_list)")
    _has_workflow   = EnkfPrototype("bool ert_workflow_list_has_workflow(ert_workflow_list, char*)")
    _get_workflow   = EnkfPrototype("workflow_ref ert_workflow_list_get_workflow(ert_workflow_list, char*)")
    _get_context    = EnkfPrototype("subst_list_ref ert_workflow_list_get_context(ert_workflow_list)")
    _add_job        = EnkfPrototype("void ert_workflow_list_add_job(ert_workflow_list, char*, char*)")
    _has_job        = EnkfPrototype("bool ert_workflow_list_has_job(ert_workflow_list, char*)")
    _get_job        = EnkfPrototype("workflow_job_ref ert_workflow_list_get_job(ert_workflow_list, char*)")
    _get_job_names  = EnkfPrototype("stringlist_obj ert_workflow_list_get_job_names(ert_workflow_list)")
    _free           = EnkfPrototype("void ert_workflow_list_free(ert_workflow_list)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def getWorkflowNames(self):
        """ @rtype: StringList """
        return self._alloc_namelist()

    def __contains__(self, workflow_name):
        assert isinstance(workflow_name, str)
        return self._has_workflow(workflow_name)

    def __getitem__(self, item):
        """ @rtype: Workflow """
        if not item in self:
            raise KeyError("Item '%s' is not in the list of available workflows." % item)

        return self._get_workflow(item).setParent(self)

    def getContext(self):
        """ @rtype: SubstitutionList """
        return self._get_context()

    def free(self):
        self._free()

    def __str__(self):
        return 'ErtWorkflowList with jobs: %s' + str(self.getJobNames())

    def addJob(self, job_name, job_path):
        """
        @type job_name: str
        @type job_path: str
        """
        self._add_job(job_name, job_path)

    def hasJob(self, job_name):
        """
         @type job_name: str
         @rtype: bool
        """
        return self._has_job(job_name)

    def getJob(self, job_name):
        """ @rtype: WorkflowJob """
        return self._get_job(job_name)

    def getJobNames(self):
        """ @rtype: StringList """
        return self._get_job_names()

    def getPluginJobs(self):
        """ @rtype: list of WorkflowJob """
        plugins = []
        for job_name in self.getJobNames():
            job = self.getJob(job_name)
            if job.isPlugin():
                plugins.append(job)
        return plugins

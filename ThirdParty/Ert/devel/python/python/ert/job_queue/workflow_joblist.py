import os
from ert.cwrap import BaseCClass, CWrapper
from ert.job_queue import JOB_QUEUE_LIB, WorkflowJob


class WorkflowJoblist(BaseCClass):

    def __init__(self):
        c_ptr = WorkflowJoblist.cNamespace().alloc()
        super(WorkflowJoblist, self).__init__(c_ptr)


    def addJob(self, job):
        """ @type job: WorkflowJob """
        job.convertToCReference(self)
        WorkflowJoblist.cNamespace().add_job(self, job)

    def addJobFromFile(self, name, filepath):
        """
         @type name: str
         @type filepath: str
         @rtype: bool
        """
        if not os.path.exists(filepath):
            raise UserWarning("Job file '%s' does not exist!" % filepath)

        return WorkflowJoblist.cNamespace().add_job_from_file(self, name, filepath)

    def __contains__(self, item):
        """
         @type item: str or WorkflowJob
         @rtype: bool
        """

        if isinstance(item, WorkflowJob):
            item = item.name()

        return WorkflowJoblist.cNamespace().has_job(self, item)


    def __getitem__(self, item):
        """
         @type item: str
         @rtype: WorkflowJob
        """

        if not item in self:
            return None

        return WorkflowJoblist.cNamespace().get_job(self, item)


    def free(self):
        WorkflowJoblist.cNamespace().free(self)


CWrapper.registerObjectType("workflow_joblist", WorkflowJoblist)

cwrapper = CWrapper(JOB_QUEUE_LIB)

WorkflowJoblist.cNamespace().alloc    = cwrapper.prototype("c_void_p workflow_joblist_alloc()")
WorkflowJoblist.cNamespace().free     = cwrapper.prototype("void     workflow_joblist_free(workflow_joblist)")

WorkflowJoblist.cNamespace().add_job  = cwrapper.prototype("void     workflow_joblist_add_job(workflow_joblist, workflow_job)")
WorkflowJoblist.cNamespace().add_job_from_file = cwrapper.prototype("bool workflow_joblist_add_job_from_file(workflow_joblist, char*, char*)")
WorkflowJoblist.cNamespace().has_job  = cwrapper.prototype("bool     workflow_joblist_has_job(workflow_joblist, char*)")
WorkflowJoblist.cNamespace().get_job  = cwrapper.prototype("workflow_job_ref workflow_joblist_get_job(workflow_joblist, char*)")
WorkflowJoblist.cNamespace().count    = cwrapper.prototype("workflow_job_ref workflow_joblist_get_job(workflow_joblist, char*)")

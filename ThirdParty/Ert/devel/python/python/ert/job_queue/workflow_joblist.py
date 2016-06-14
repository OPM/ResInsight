import os
from ert.cwrap import BaseCClass
from ert.job_queue import QueuePrototype, WorkflowJob


class WorkflowJoblist(BaseCClass):
    TYPE_NAME = "workflow_joblist"
    _alloc              = QueuePrototype("void*            workflow_joblist_alloc()" , bind = False)
    _free               = QueuePrototype("void             workflow_joblist_free(workflow_joblist)")
    _add_job            = QueuePrototype("void             workflow_joblist_add_job(workflow_joblist, workflow_job)")
    _add_job_from_file  = QueuePrototype("bool             workflow_joblist_add_job_from_file(workflow_joblist, char*, char*)")
    _has_job            = QueuePrototype("bool             workflow_joblist_has_job(workflow_joblist, char*)")
    _get_job            = QueuePrototype("workflow_job_ref workflow_joblist_get_job(workflow_joblist, char*)")
    _count              = QueuePrototype("workflow_job_ref workflow_joblist_get_job(workflow_joblist, char*)")

    def __init__(self):
        c_ptr = self._alloc( )
        super(WorkflowJoblist, self).__init__(c_ptr)


    def addJob(self, job):
        """ @type job: WorkflowJob """
        job.convertToCReference(self)
        self._add_job(job)

        
    def addJobFromFile(self, name, filepath):
        """
         @type name: str
         @type filepath: str
         @rtype: bool
        """
        if not os.path.exists(filepath):
            raise UserWarning("Job file '%s' does not exist!" % filepath)

        return self._add_job_from_file(name, filepath)

    
    def __contains__(self, item):
        """
         @type item: str or WorkflowJob
         @rtype: bool
        """

        if isinstance(item, WorkflowJob):
            item = item.name()

        return self._has_job(item)


    def __getitem__(self, item):
        """
         @type item: str
         @rtype: WorkflowJob
        """

        if not item in self:
            return None

        return self._get_job(item)


    
    def free(self):
        self._free( )


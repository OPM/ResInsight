import time
from ert.config import ConfigError
from cwrap import BaseCClass
from ert.job_queue import QueuePrototype, WorkflowJoblist, WorkflowJob
from ert.util import SubstitutionList


class Workflow(BaseCClass):
    TYPE_NAME = "workflow"
    _alloc          = QueuePrototype("void* workflow_alloc(char*, workflow_joblist)" , bind = False)
    _free           = QueuePrototype("void     workflow_free(workflow)")
    _count          = QueuePrototype("int      workflow_size(workflow)")
    _iget_job       = QueuePrototype("workflow_job_ref workflow_iget_job(workflow, int)")
    _iget_args      = QueuePrototype("stringlist_ref   workflow_iget_arguments(workflow, int)")

    _try_compile    = QueuePrototype("bool workflow_try_compile(workflow, subst_list)")
    _get_last_error = QueuePrototype("config_error_ref workflow_get_last_error(workflow)")

    def __init__(self, src_file, job_list):
        """
        @type src_file: str
        @type job_list: WorkflowJoblist
        """
        c_ptr = self._alloc(src_file, job_list)
        super(Workflow, self).__init__(c_ptr)

        self.__running = False
        self.__cancelled = False
        self.__current_job = None

    def __len__(self):
        return self._count( )

    def __getitem__(self, index):
        """
        @type index: int
        @rtype: tuple of (WorkflowJob, arguments)
        """
        job = self._iget_job(index)
        args = self._iget_args(index)
        return job, args

    
    def __iter__(self):
        for index in range(len(self)):
            yield self[index]

    def run(self, ert, verbose=False, context=None):
        """
        @type ert: ert.enkf.enkf_main.EnKFMain
        @type verbose: bool
        @type context: SubstitutionList
        @rtype: bool
        """
        self.__running = True
        success = self._try_compile(context)

        if success:
            for job, args in self:
                self.__current_job = job
                if not self.__cancelled:
                    return_value = job.run(ert, args, verbose)
                    
                    if job.hasFailed():
                        print(return_value)

                    #todo store results?

        self.__current_job = None
        self.__running = False
        return success


    def free(self):
        self._free( )

    def isRunning(self):
        return self.__running

    def cancel(self):
        if self.__current_job is not None:
            self.__current_job.cancel()

        self.__cancelled = True

    def isCancelled(self):
        return self.__cancelled

    def wait(self):
        while self.isRunning():
            time.sleep(1)

    def getLastError(self):
        """ @rtype: ConfigError """
        return self._get_last_error( )

    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        workflow = super(Workflow, cls).createCReference(c_pointer, parent)
        workflow.__running = False
        workflow.__cancelled = False
        workflow.__current_job = None
        return workflow



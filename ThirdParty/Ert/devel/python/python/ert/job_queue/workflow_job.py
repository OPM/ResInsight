from ert.cwrap import BaseCClass, CWrapper
from ert.job_queue import JOB_QUEUE_LIB, ErtScript, FunctionErtScript, ErtPlugin, ExternalErtScript
from ert.config import ContentTypeEnum


class WorkflowJob(BaseCClass):

    def __init__(self, name, internal=True):
        c_ptr = WorkflowJob.cNamespace().alloc(name, internal)
        super(WorkflowJob, self).__init__(c_ptr)

        self.__script = None
        """ :type: ErtScript """
        self.__running = False

    def isInternal(self):
        """ @rtype: bool """
        return WorkflowJob.cNamespace().internal(self)

    def name(self):
        """ @rtype: str """
        return WorkflowJob.cNamespace().name(self)

    def minimumArgumentCount(self):
        """ @rtype: int """
        return WorkflowJob.cNamespace().min_arg(self)

    def maximumArgumentCount(self):
        """ @rtype: int """
        return WorkflowJob.cNamespace().max_arg(self)

    def functionName(self):
        """ @rtype: str """
        return WorkflowJob.cNamespace().get_function(self)

    def module(self):
        """ @rtype: str """
        return WorkflowJob.cNamespace().get_module(self)

    def executable(self):
        """ @rtype: str """
        return WorkflowJob.cNamespace().get_executable(self)

    def isInternalScript(self):
        """ @rtype: bool """
        return WorkflowJob.cNamespace().is_internal_script(self)

    def getInternalScriptPath(self):
        """ @rtype: str """
        return WorkflowJob.cNamespace().get_internal_script(self)

    def isPlugin(self):
        """ @rtype: bool """
        if self.isInternalScript():
            script_obj = ErtScript.loadScriptFromFile(self.getInternalScriptPath())
            return script_obj is not None and issubclass(script_obj, ErtPlugin)

        return False


    def argumentTypes(self):
        """ @rtype: list of type """

        result = []
        for index in range(self.maximumArgumentCount()):
            t = WorkflowJob.cNamespace().arg_type(self, index)
            if t == ContentTypeEnum.CONFIG_BOOL:
                result.append(bool)
            elif t == ContentTypeEnum.CONFIG_FLOAT:
                result.append(float)
            elif t == ContentTypeEnum.CONFIG_INT:
                result.append(int)
            elif t == ContentTypeEnum.CONFIG_STRING:
                result.append(str)
            else:
                result.append(None)

        return result


    def run(self, ert, arguments, verbose=False):
        """
        @type ert: ert.enkf.enkf_main.EnKFMain
        @type arguments: list of str
        @type verbose: bool
        @rtype: ctypes.c_void_p
        """
        self.__running = True

        min_arg = self.minimumArgumentCount()
        if min_arg > 0 and len(arguments) < min_arg:
            raise UserWarning("The job: %s requires at least %d arguments, %d given." % (self.name(), min_arg, len(arguments)))

        max_arg = self.maximumArgumentCount()
        if 0 < max_arg < len(arguments):
            raise UserWarning("The job: %s can only have %d arguments, %d given." % (self.name(), max_arg, len(arguments)))


        if self.isInternalScript():
            script_obj = ErtScript.loadScriptFromFile(self.getInternalScriptPath())
            self.__script = script_obj(ert)
            result = self.__script.initializeAndRun(self.argumentTypes(), arguments, verbose=verbose)

        elif self.isInternal() and not self.isInternalScript():
            self.__script = FunctionErtScript(ert, self.functionName(), self.argumentTypes(), argument_count=len(arguments))
            result = self.__script.initializeAndRun(self.argumentTypes(), arguments, verbose=verbose)

        elif not self.isInternal():
            self.__script = ExternalErtScript(ert, self.executable())
            result = self.__script.initializeAndRun(self.argumentTypes(), arguments, verbose=verbose)

        else:
            raise UserWarning("Unknown script type!")

        self.__running = False
        return result

    def cancel(self):
        if self.__script is not None:
            self.__script.cancel()

    def isRunning(self):
        return self.__running

    def isCancelled(self):
        return self.__script.isCancelled()

    def hasFailed(self):
        """ @rtype: bool """
        return self.__script.hasFailed()

    def free(self):
        WorkflowJob.cNamespace().free(self)


    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        workflow = super(WorkflowJob, cls).createCReference(c_pointer, parent)
        workflow.__script = None
        workflow.__running = False
        return workflow


CWrapper.registerObjectType("workflow_job", WorkflowJob)

cwrapper = CWrapper(JOB_QUEUE_LIB)

WorkflowJob.cNamespace().alloc    = cwrapper.prototype("c_void_p workflow_job_alloc(char*, bool)")
WorkflowJob.cNamespace().free     = cwrapper.prototype("void     workflow_job_free(workflow_job)")
WorkflowJob.cNamespace().name     = cwrapper.prototype("char*    workflow_job_get_name(workflow_job)")
WorkflowJob.cNamespace().internal = cwrapper.prototype("bool     workflow_job_internal(workflow_job)")
WorkflowJob.cNamespace().is_internal_script  = cwrapper.prototype("bool   workflow_job_is_internal_script(workflow_job)")
WorkflowJob.cNamespace().get_internal_script = cwrapper.prototype("char*  workflow_job_get_internal_script_path(workflow_job)")
WorkflowJob.cNamespace().get_function   = cwrapper.prototype("char*  workflow_job_get_function(workflow_job)")
WorkflowJob.cNamespace().get_module     = cwrapper.prototype("char*  workflow_job_get_module(workflow_job)")
WorkflowJob.cNamespace().get_executable = cwrapper.prototype("char*  workflow_job_get_executable(workflow_job)")

WorkflowJob.cNamespace().min_arg  = cwrapper.prototype("int  workflow_job_get_min_arg(workflow_job)")
WorkflowJob.cNamespace().max_arg  = cwrapper.prototype("int  workflow_job_get_max_arg(workflow_job)")
WorkflowJob.cNamespace().arg_type = cwrapper.prototype("config_content_type_enum workflow_job_iget_argtype(workflow_job, int)")


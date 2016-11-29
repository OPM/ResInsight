import os

from cwrap import BaseCClass, CWrapper
from ert.job_queue import QueuePrototype, ErtScript, FunctionErtScript, ErtPlugin, ExternalErtScript
from ert.config import ContentTypeEnum


class WorkflowJob(BaseCClass):
    TYPE_NAME = "workflow_job"
    _alloc               = QueuePrototype("void* workflow_job_alloc(char*, bool)", bind= False)
    _alloc_parser        = QueuePrototype("config_parser_obj workflow_job_alloc_config( )", bind= False)
    _alloc_from_file     = QueuePrototype("workflow_job_obj workflow_job_config_alloc( char* , config_parser , char*)", bind= False)
    _free                = QueuePrototype("void     workflow_job_free(workflow_job)")
    _name                = QueuePrototype("char*    workflow_job_get_name(workflow_job)")
    _internal            = QueuePrototype("bool     workflow_job_internal(workflow_job)")
    _is_internal_script  = QueuePrototype("bool   workflow_job_is_internal_script(workflow_job)")
    _get_internal_script = QueuePrototype("char*  workflow_job_get_internal_script_path(workflow_job)")
    _get_function        = QueuePrototype("char*  workflow_job_get_function(workflow_job)")
    _get_module          = QueuePrototype("char*  workflow_job_get_module(workflow_job)")
    _get_executable      = QueuePrototype("char*  workflow_job_get_executable(workflow_job)")
    _min_arg             = QueuePrototype("int  workflow_job_get_min_arg(workflow_job)")
    _max_arg             = QueuePrototype("int  workflow_job_get_max_arg(workflow_job)")
    _arg_type            = QueuePrototype("config_content_type_enum workflow_job_iget_argtype(workflow_job, int)")



    @classmethod
    def configParser(cls):
        return cls._alloc_parser( )

    @classmethod
    def fromFile(cls , config_file , name = None , parser = None):
        if os.path.isfile( config_file ) and os.access( config_file , os.R_OK):
            if parser is None:
                parser = cls.configParser( )

            if name is None:
                name = os.path.basename( config_file )

            # NB: Observe argument reoredring.
            return cls._alloc_from_file( name , parser , config_file )
        else:
            raise IOError("Could not open config_file:%s" % config_file)   
    
    
    def __init__(self, name, internal=True):
        c_ptr = self._alloc(name, internal)
        super(WorkflowJob, self).__init__(c_ptr)

        self.__script = None
        """ :type: ErtScript """
        self.__running = False

    def isInternal(self):
        """ @rtype: bool """
        return self._internal( )

    def name(self):
        """ @rtype: str """
        return self._name()

    def minimumArgumentCount(self):
        """ @rtype: int """
        return self._min_arg()

    def maximumArgumentCount(self):
        """ @rtype: int """
        return self._max_arg( )

    def functionName(self):
        """ @rtype: str """
        return self._get_function( )

    def module(self):
        """ @rtype: str """
        return self._get_module( )

    def executable(self):
        """ @rtype: str """
        return self._get_executable( )

    def isInternalScript(self):
        """ @rtype: bool """
        return self._is_internal_script( )

    def getInternalScriptPath(self):
        """ @rtype: str """
        return self._get_internal_script( )

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
            t = self._arg_type(index)
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
        self._free( )


    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        workflow = super(WorkflowJob, cls).createCReference(c_pointer, parent)
        workflow.__script = None
        workflow.__running = False
        return workflow

    

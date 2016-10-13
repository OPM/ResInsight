from ert.job_queue import ErtScript, ErtPlugin, WorkflowJob

class Plugin(object):
    def __init__(self, ert, workflow_job):
        """
        @type ert: ert.enkf.EnKFMain
        @type workflow_job: WorkflowJob
        """
        self.__ert = ert
        self.__workflow_job = workflow_job
        self.__parent_window = None

        script = self.__loadPlugin()
        self.__name = script.getName()
        self.__description = script.getDescription()

    
    def __loadPlugin(self):
        """ @rtype: ErtPlugin """
        script_obj = ErtScript.loadScriptFromFile(self.__workflow_job.getInternalScriptPath())
        script = script_obj(self.__ert)
        return script

    def getName(self):
        """ @rtype: str """
        return self.__name

    def getDescription(self):
        """ @rtype: str """
        return self.__description

    def getArguments(self):
        """
         Returns a list of arguments. Either from GUI or from arbitrary code.
         If the user for example cancels in the GUI a CancelPluginException is raised.
        @rtype: list """
        script = self.__loadPlugin()
        return script.getArguments(self.__parent_window)


    def setParentWindow(self, parent_window):
        self.__parent_window = parent_window

    def getParentWindow(self):
        """ @rtype: QWidget """
        return self.__parent_window

    def ert(self):
        """ @rtype: ert.enkf.enkf_main.EnKFMain """
        return self.__ert

    def getWorkflowJob(self):
        """ @rtype: WorkflowJob """
        return self.__workflow_job

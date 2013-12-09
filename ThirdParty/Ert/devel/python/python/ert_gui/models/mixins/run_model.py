from ert.job_queue.job_status_type_enum import JobStatusType
from ert_gui.models.mixins import ModelMixin, AbstractMethodError

class ErtRunError(Exception):
    pass

class RunModelMixin(ModelMixin):

    def startSimulations(self):
        raise AbstractMethodError(self, "startSimulations")

    def runSimulations(self):
        raise AbstractMethodError(self, "runSimulations")

    def killAllSimulations(self):
        raise AbstractMethodError(self, "killAllSimulations")

    def phaseCount(self):
        """ @rtype: int """
        raise AbstractMethodError(self, "phaseCount")

    def setPhaseCount(self, phase_count):
        raise AbstractMethodError(self, "setPhaseCount")

    def currentPhase(self):
        """ @rtype: int """
        raise AbstractMethodError(self, "currentPhase")

    def setPhaseName(self, phase_name):
        raise AbstractMethodError(self, "setPhaseName")

    def getPhaseName(self):
        """ @rtype: str """
        raise AbstractMethodError(self, "getPhaseName")

    def setIndeterminate(self, indeterminate):
        raise AbstractMethodError(self, "setIndeterminate")

    def isFinished(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "isFinished")

    def hasRunFailed(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "hasRunFailed")

    def getFailMessage(self):
        """ @rtype: str """
        raise AbstractMethodError(self, "getFailMessage")

    def setPhase(self, phase, phase_name):
        raise AbstractMethodError(self, "setPhase")

    def getRunningTime(self):
        """ @rtype: int """
        raise AbstractMethodError(self, "getRunningTime")

    def getQueueSize(self):
        """ @rtype: int """
        raise AbstractMethodError(self, "getQueueSize")

    def getQueueStatus(self):
        """ @rtype: dict of (JobStatusType, int) """
        raise AbstractMethodError(self, "getQueueStatus")

    def isQueueRunning(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "isQueueRunning")

    def getProgress(self):
        """ @rtype: float """
        raise AbstractMethodError(self, "getProgress")

    def isIndeterminate(self):
        """ @rtype: bool """
        raise AbstractMethodError(self, "isIndeterminate")


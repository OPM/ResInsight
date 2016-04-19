import time
from ert.job_queue import JobStatusType
from ert_gui.models import ErtConnector
from ert_gui.models.mixins import RunModelMixin, ErtRunError, AbstractMethodError


class BaseRunModel(ErtConnector, RunModelMixin):

    def __init__(self, name, phase_count=1, *args):
        super(BaseRunModel, self).__init__(*args)
        self.__name = name
        self.__phase = 0
        self.__phase_count = phase_count
        self.__phase_update_count = 0
        self.__phase_name = "Not defined"

        self.__job_start_time  = 0
        self.__job_stop_time = 0
        self.__indeterminate = False
        self.__fail_message = ""
        self.reset( )



    def reset(self):
        self.__failed = False


    def startSimulations(self):
        try:
            self.runSimulations()
        except ErtRunError as e:
            self.__failed = True
            self.__fail_message = str(e)
            self.__simulationEnded()


    def runSimulations(self):
        raise AbstractMethodError(self, "runSimulations")


    def killAllSimulations(self):
        job_queue = self.ert().siteConfig().getJobQueue()
        job_queue.killAllJobs()


    def phaseCount(self):
        """ @rtype: int """
        return self.__phase_count


    def setPhaseCount(self, phase_count):
        self.__phase_count = phase_count
        self.setPhase(0, "")


    def currentPhase(self):
        """ @rtype: int """
        return self.__phase


    def setPhaseName(self, phase_name, indeterminate=None):
        self.__phase_name = phase_name
        self.setIndeterminate(indeterminate)


    def getPhaseName(self):
        """ @rtype: str """
        return self.__phase_name


    def setIndeterminate(self, indeterminate):
        if indeterminate is not None:
            self.__indeterminate = indeterminate


    def isFinished(self):
        """ @rtype: bool """
        return self.__phase == self.__phase_count or self.hasRunFailed()


    def hasRunFailed(self):
        return self.__failed


    def getFailMessage(self):
        """ @rtype: str """
        return self.__fail_message


    def __simulationEnded(self):
        self.__job_stop_time = int(time.time())


    def setPhase(self, phase, phase_name, indeterminate=None):
        self.setPhaseName(phase_name)
        if not 0 <= phase <= self.__phase_count:
            raise ValueError("Phase must be an integer from 0 to less than %d." % self.__phase_count)

        self.setIndeterminate(indeterminate)

        if phase == 0:
            self.__job_start_time = int(time.time())

        if phase == self.__phase_count:
            self.__simulationEnded()

        self.__phase = phase
        self.__phase_update_count = 0


    def getRunningTime(self):
        if self.__job_stop_time < self.__job_start_time:
            return time.time() - self.__job_start_time
        else:
            return self.__job_stop_time - self.__job_start_time


    def getQueueSize(self):
        queue_size = len(self.ert().siteConfig().getJobQueue())

        if queue_size == 0:
            queue_size = 1

        return queue_size


    def getQueueStatus(self):
        """ @rtype: dict of (JobStatusType, int) """
        job_queue = self.ert().siteConfig().getJobQueue()

        queue_status = {}

        if job_queue.isRunning():
            for job_number in range(len(job_queue)):
                status = job_queue.getJobStatus(job_number)

                if not status in queue_status:
                    queue_status[status] = 0

                queue_status[status] += 1

        return queue_status

    def isQueueRunning(self):
        """ @rtype: bool """
        return self.ert().siteConfig().getJobQueue().isRunning()


    def getProgress(self):
        """ @rtype: float """
        if self.isFinished():
            current_progress = 1.0
        elif not self.isQueueRunning() and self.__phase_update_count > 0:
            current_progress = (self.__phase + 1.0) / self.__phase_count
        else:
            self.__phase_update_count += 1
            queue_status = self.getQueueStatus()
            queue_size = self.getQueueSize()

            done_state = JobStatusType.JOB_QUEUE_SUCCESS | JobStatusType.JOB_QUEUE_DONE
            done_count = 0

            for state in queue_status:
                if state in done_state:
                    done_count += queue_status[state]

            phase_progress = float(done_count) / queue_size
            current_progress = (self.__phase + phase_progress) / self.__phase_count

        return current_progress


    def isIndeterminate(self):
        """ @rtype: bool """
        return not self.isFinished() and self.__indeterminate


    def __str__(self):
        return self.__name



import time
from ert.job_queue import JobStatusType
from ert_gui import ERT
from ert.enkf import ErtLog


class ErtRunError(Exception):
    pass

class BaseRunModel(object):

    def __init__(self, name, phase_count=1):
        super(BaseRunModel, self).__init__()
        self._name = name
        self._phase = 0
        self._phase_count = phase_count
        self._phase_update_count = 0
        self._phase_name = "Not defined"

        self._job_start_time  = 0
        self._job_stop_time = 0
        self._indeterminate = False
        self._fail_message = ""
        self._failed = False
        self.reset( )


    def ert(self):
        """ @rtype: ert.enkf.EnKFMain"""
        return ERT.ert

    def reset(self):
        self._failed = False


    def startSimulations(self, run_arguments):
        try:
            self.runSimulations(run_arguments)
        except ErtRunError as e:
            self._failed = True
            self._fail_message = str(e)
            self._simulationEnded()


    def runSimulations(self, run_arguments):
        raise NotImplementedError("Method must be implemented by inheritors!")


    def killAllSimulations(self):
        job_queue = self.ert().siteConfig().getJobQueue()
        job_queue.killAllJobs()


    def userExitCalled(self):
        """ @rtype: bool """
        job_queue = self.ert().siteConfig().getJobQueue()
        return job_queue.getUserExit( )


    def phaseCount(self):
        """ @rtype: int """
        return self._phase_count


    def setPhaseCount(self, phase_count):
        self._phase_count = phase_count
        self.setPhase(0, "")


    def currentPhase(self):
        """ @rtype: int """
        return self._phase


    def setPhaseName(self, phase_name, indeterminate=None):
        self._phase_name = phase_name
        self.setIndeterminate(indeterminate)


    def getPhaseName(self):
        """ @rtype: str """
        return self._phase_name


    def setIndeterminate(self, indeterminate):
        if indeterminate is not None:
            self._indeterminate = indeterminate


    def isFinished(self):
        """ @rtype: bool """
        return self._phase == self._phase_count or self.hasRunFailed()


    def hasRunFailed(self):
        """ @rtype: bool """
        return self._failed


    def getFailMessage(self):
        """ @rtype: str """
        return self._fail_message


    def _simulationEnded(self):
        self._job_stop_time = int(time.time())


    def setPhase(self, phase, phase_name, indeterminate=None):
        self.setPhaseName(phase_name)
        if not 0 <= phase <= self._phase_count:
            raise ValueError("Phase must be an integer from 0 to less than %d." % self._phase_count)

        self.setIndeterminate(indeterminate)

        if phase == 0:
            self._job_start_time = int(time.time())

        if phase == self._phase_count:
            self._simulationEnded()

        self._phase = phase
        self._phase_update_count = 0


    def getRunningTime(self):
        """ @rtype: float """
        if self._job_stop_time < self._job_start_time:
            return time.time() - self._job_start_time
        else:
            return self._job_stop_time - self._job_start_time


    def getQueueSize(self):
        """ @rtype: int """
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
        elif not self.isQueueRunning() and self._phase_update_count > 0:
            current_progress = (self._phase + 1.0) / self._phase_count
        else:
            self._phase_update_count += 1
            queue_status = self.getQueueStatus()
            queue_size = self.getQueueSize()

            done_state = JobStatusType.JOB_QUEUE_SUCCESS | JobStatusType.JOB_QUEUE_DONE
            done_count = 0

            for state in queue_status:
                if state in done_state:
                    done_count += queue_status[state]

            phase_progress = float(done_count) / queue_size
            current_progress = (self._phase + phase_progress) / self._phase_count

        return current_progress


    def isIndeterminate(self):
        """ @rtype: bool """
        return not self.isFinished() and self._indeterminate


    def checkHaveSufficientRealizations(self, num_successful_realizations):
        if num_successful_realizations == 0:
            raise ErtRunError("Simulation failed! All realizations failed!")
        elif not self.ert().analysisConfig().haveEnoughRealisations(num_successful_realizations, self.ert().getEnsembleSize()):
            raise ErtRunError("Too many simulations have failed! You can add/adjust MIN_REALIZATIONS to allow failures in your simulations.\n\n"
                              "Check ERT log file '%s' or simulation folder for details." % ErtLog.getFilename())


    def __str__(self):
        return self._name



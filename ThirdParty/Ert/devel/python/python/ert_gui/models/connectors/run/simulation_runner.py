from threading import Thread
import time
from ert_gui.models.connectors.run.run_status import RunStatusModel
from ert_gui.models.mixins import ModelMixin, RunModelMixin


class SimulationRunner(Thread, ModelMixin):
    SIMULATION_FINISHED_EVENT = "simulation_finished_event"

    def __init__(self, button_model):
        super(SimulationRunner, self).__init__(name="enkf_main_run_thread")
        self.setDaemon(True)

        assert isinstance(button_model, RunModelMixin)

        self.__model = button_model

        self.__job_start_time  = 0
        self.__job_stop_time = 0

    def registerDefaultEvents(self):
        super(SimulationRunner, self).registerDefaultEvents()
        self.observable().addEvent(SimulationRunner.SIMULATION_FINISHED_EVENT)


    def run(self):
        self.__job_start_time = int(time.time())

        status_thread = Thread(name = "enkf_main_run_status_poll_thread")
        status_thread.setDaemon(True)
        status_thread.run = RunStatusModel().startStatusPoller
        status_thread.start()

        self.__model.startSimulations()
        self.__job_stop_time = int(time.time())

        self.observable().notify(SimulationRunner.SIMULATION_FINISHED_EVENT)

    def getRunningTime(self):
        if self.__job_stop_time < self.__job_start_time:
            return time.time() - self.__job_start_time
        else:
            return self.__job_stop_time - self.__job_start_time

    def killAllSimulations(self):
        self.__model.killAllSimulations()




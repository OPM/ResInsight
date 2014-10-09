from ert.job_queue import JobStatusType
from ert_gui.models.mixins import ListModelMixin


class SimulationStateStatus(object):
    COLOR_WAITING = (164, 164, 255)
    COLOR_PENDING = (164, 200, 255)
    COLOR_RUNNING = (200, 255, 200)
    COLOR_FAILED  = (255, 200, 200)

    COLOR_USER_KILLED = (255, 255, 200)
    COLOR_FINISHED   = (200, 200, 200)
    COLOR_NOT_ACTIVE  = (255, 255, 255)

    def __init__(self, name, state, color):
        self.__name = name
        self.__state = state
        self.__color = color

        self.__count = 0
        self.__total_count = 1

    @property
    def name(self):
        return self.__name

    @property
    def state(self):
        return self.__state

    @property
    def color(self):
        return self.__color

    @property
    def count(self):
        return self.__count

    @count.setter
    def count(self, value):
        self.__count = value

    @property
    def total_count(self):
        return self.__total_count

    @total_count.setter
    def total_count(self, value):
        self.__total_count = value


class SimulationsTracker(ListModelMixin):
    def __init__(self):
        super(SimulationsTracker, self).__init__()

        waiting_state = SimulationStateStatus("Waiting", JobStatusType.JOB_QUEUE_NOT_ACTIVE | JobStatusType.JOB_QUEUE_WAITING | JobStatusType.JOB_QUEUE_SUBMITTED, SimulationStateStatus.COLOR_WAITING)
        pending_state = SimulationStateStatus("Pending", JobStatusType.JOB_QUEUE_PENDING, SimulationStateStatus.COLOR_PENDING)
        running_state = SimulationStateStatus("Running", JobStatusType.JOB_QUEUE_RUNNING | JobStatusType.JOB_QUEUE_EXIT | JobStatusType.JOB_QUEUE_RUNNING_CALLBACK, SimulationStateStatus.COLOR_RUNNING)
        killed_state = SimulationStateStatus("User killed", JobStatusType.JOB_QUEUE_USER_KILLED | JobStatusType.JOB_QUEUE_USER_EXIT, SimulationStateStatus.COLOR_USER_KILLED)
        failed_state = SimulationStateStatus("Failed", JobStatusType.JOB_QUEUE_FAILED, SimulationStateStatus.COLOR_FAILED)
        done_state = SimulationStateStatus("Finished", JobStatusType.JOB_QUEUE_DONE | JobStatusType.JOB_QUEUE_SUCCESS, SimulationStateStatus.COLOR_FINISHED)

        self.states = [waiting_state, pending_state, running_state, killed_state, failed_state, done_state]
        self.custom_states = [waiting_state, pending_state, running_state, failed_state, done_state]

        self.__checkForUnusedEnums()

    def getList(self):
        """ @rtype: list of SimulationStateStatus """
        return list(self.custom_states)

    def __checkForUnusedEnums(self):
        for enum in JobStatusType.enums():
            used = False
            for state in self.states:
                if enum in state.state:
                    used = True

            if not used:
                raise AssertionError("Enum identifier '%s' not used!" % enum)








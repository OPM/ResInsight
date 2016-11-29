from ert.job_queue import JobStatusType

class SimulationStateStatus(object):
    COLOR_WAITING = (164, 164, 255)
    COLOR_PENDING = (164, 200, 255)
    COLOR_RUNNING = (200, 255, 200)
    COLOR_FAILED  = (255, 200, 200)

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


class SimulationsTracker(object):
    def __init__(self):
        super(SimulationsTracker, self).__init__()

        waiting_flag  = JobStatusType.JOB_QUEUE_NOT_ACTIVE | JobStatusType.JOB_QUEUE_WAITING | JobStatusType.JOB_QUEUE_SUBMITTED
        waiting_state = SimulationStateStatus("Waiting", waiting_flag, SimulationStateStatus.COLOR_WAITING)

        pending_flag  = JobStatusType.JOB_QUEUE_PENDING
        pending_state = SimulationStateStatus("Pending", pending_flag, SimulationStateStatus.COLOR_PENDING)

        running_flag  = JobStatusType.JOB_QUEUE_RUNNING | JobStatusType.JOB_QUEUE_EXIT | JobStatusType.JOB_QUEUE_RUNNING_CALLBACK
        running_state = SimulationStateStatus("Running", running_flag, SimulationStateStatus.COLOR_RUNNING)

        # Failed also includes simulations which have been killed by the MAX_RUNTIME system.
        failed_flag  = JobStatusType.JOB_QUEUE_IS_KILLED | JobStatusType.JOB_QUEUE_DO_KILL
        failed_flag |= JobStatusType.JOB_QUEUE_FAILED    | JobStatusType.JOB_QUEUE_DO_KILL_NODE_FAILURE
        failed_state = SimulationStateStatus("Failed", failed_flag, SimulationStateStatus.COLOR_FAILED)

        done_flag  = JobStatusType.JOB_QUEUE_DONE | JobStatusType.JOB_QUEUE_SUCCESS
        done_state = SimulationStateStatus("Finished", done_flag, SimulationStateStatus.COLOR_FINISHED)

        self.states = [waiting_state, pending_state, running_state, failed_state, done_state]
        self.custom_states = [waiting_state, pending_state, running_state, failed_state, done_state]

        self.__checkForUnusedEnums()

    def getStates(self):
        """ @rtype: list[SimulationStateStatus] """
        return list(self.custom_states)

    def __checkForUnusedEnums(self):
        for enum in JobStatusType.enums():
            used = False
            for state in self.states:
                if enum in state.state:
                    used = True

            if not used:
                raise AssertionError("Enum identifier '%s' not used!" % enum)

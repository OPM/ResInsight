import time
from ert_gui.models import ErtConnector
from ert_gui.models.connectors.run import RunMembersModel
from ert_gui.models.mixins import ModelMixin


class RunStatusModel(ErtConnector, ModelMixin):
    STATUS_CHANGED_EVENT = "status_changed_event"

    def __init__(self):
        super(RunStatusModel, self).__init__()

        self.__status = {}
        self.__resetStatusFlag()
        self.__status_count = {}

        self.__is_running = False


    def registerDefaultEvents(self):
        super(RunStatusModel, self).registerDefaultEvents()
        self.observable().addEvent(RunStatusModel.STATUS_CHANGED_EVENT)


    def startStatusPoller(self):
        assert not self.__is_running, "Job already started!"

        while not self.ert().siteConfig().isQueueRunning():
            time.sleep(0.5)

        self.__is_running = True

        selected_members = [int(member) for member in RunMembersModel().getSelectedItems()]
        self.__resetMemberStatus()

        while self.ert().siteConfig().isQueueRunning():
            self.__resetStatusFlag()

            for member in selected_members:
                state = self.ert().getMemberRunningState(member)
                status = state.getRunStatus()

                self.__setMemberStatus(member, status)

            self.__updateStatusCount()

            self.__checkStatusChangedFlag() # Emit once for all members if any changes has occurred

            time.sleep(0.5)

        self.__is_running = False


    def getStatusCounts(self):
        return dict(self.__status_count)

    def __setMemberStatus(self, member, status):
        if not self.__status.has_key(member):
            self.__status[member] = status
            self.__setStatusChangedFlag()

        if self.__status[member] != status:
            self.__status[member] = status
            self.__setStatusChangedFlag()

    def __resetStatusFlag(self):
        self.__status_changed_flag = False

    def __setStatusChangedFlag(self):
        self.__status_changed_flag = True

    def __checkStatusChangedFlag(self):
        if self.__status_changed_flag:
            self.observable().notify(RunStatusModel.STATUS_CHANGED_EVENT)

    def __resetMemberStatus(self):
        self.__status.clear()

    def __updateStatusCount(self):
        self.__status_count.clear()

        for member in self.__status:
            status = self.__status[member]

            if not self.__status_count.has_key(status):
                self.__status_count[status] = 0

            self.__status_count[status] += 1


from ert_gui.models import ErtConnector
from ert_gui.models.connectors.init.case_list import CaseList
from ert_gui.models.mixins import ListModelMixin
from ert.enkf.util import TimeMap


class ReportStepsModel(ErtConnector, ListModelMixin):

    def __init__(self):
        self.__time_map = None
        self.__report_steps = None
        super(ReportStepsModel, self).__init__()


    def getReportSteps(self):
        """ @rtype: TimeMap """
        if self.__time_map is None:
            case_list = CaseList().getAllCasesWithDataAndNotRunning()

            for case in case_list:
                time_map = self.ert().getEnkfFsManager().getTimeMapForCase(case)

                if len(time_map) > 0:
                    self.__time_map = time_map

            if self.__time_map is None:
                self.__time_map = []

        return self.__time_map


    def getList(self):
        """ @rtype: list of CTime """

        time_list = [c_time for c_time in self.getReportSteps()]
        if len(time_list) > 0:
            time_list = time_list[1:]
        return time_list




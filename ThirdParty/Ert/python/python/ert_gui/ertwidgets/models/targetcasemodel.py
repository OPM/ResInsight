from ert_gui import ERT
from ert_gui.ertwidgets.models.ertmodel import getCurrentCaseName
from ert_gui.ertwidgets.models.valuemodel import ValueModel


class TargetCaseModel(ValueModel):
    def __init__(self, format_mode=False):
        self._format_mode = format_mode
        self._custom = False
        ValueModel.__init__(self, self.getDefaultValue())
        ERT.ertChanged.connect(self._caseChanged)

    def setValue(self, target_case):
        if target_case is None or target_case.strip() == "" or target_case == self.getDefaultValue():
            self._custom = False
            ValueModel.setValue(self, self.getDefaultValue())
        else:
            self._custom = True
            ValueModel.setValue(self, target_case)

    def getDefaultValue(self):
        """ @rtype: str """
        if self._format_mode:
            if ERT.ert.analysisConfig().getAnalysisIterConfig().caseFormatSet():
                return ERT.ert.analysisConfig().getAnalysisIterConfig().getCaseFormat()
            else:
                case_name = getCurrentCaseName()
                return "%s_%%d" % case_name
        else:
            case_name = getCurrentCaseName()
            return "%s_smoother_update" % case_name


    def _caseChanged(self):
        if not self._custom:
            ValueModel.setValue(self, self.getDefaultValue())

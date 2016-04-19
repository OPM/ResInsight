



from ert.analysis.enums.analysis_module_options_enum import AnalysisModuleOptionsEnum
from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ChoiceModelMixin


class IteratedAnalysisModuleModel(ErtConnector, ChoiceModelMixin):

    def __init__(self):
        self.__value = None
        super(IteratedAnalysisModuleModel, self).__init__()

    def getChoices(self):
        modules = self.ert().analysisConfig().getModuleList()

        iterable = []
        for module_name in modules:
            module = self.ert().analysisConfig().getModule(module_name)
            is_iterable = module.checkOption(AnalysisModuleOptionsEnum.ANALYSIS_ITERABLE)
            if is_iterable:
                iterable.append(module_name)

        return sorted(iterable)

    def getCurrentChoice(self):
        if self.__value is None:
            active_name = self.ert().analysisConfig().activeModuleName()
            modules = self.getChoices()
            if active_name in modules:
                self.__value = active_name
            elif "RML_ENKF" in modules:
                self.__value = "RML_ENKF"
            else:
                if len(modules) > 0:
                    self.__value = modules[0]
        return self.__value

    def setCurrentChoice(self, value):
        self.__value = value
        self.observable().notify(self.CURRENT_CHOICE_CHANGED_EVENT)

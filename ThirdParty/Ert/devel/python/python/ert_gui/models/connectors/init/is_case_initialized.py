from ert_gui.models import ErtConnector
from ert_gui.models.connectors.ensemble_resizer import EnsembleSizeModel
from ert_gui.models.connectors.init import InitializeFromScratchModel, CaseSelectorModel
from ert_gui.models.connectors.init.init_from_existing import InitializeFromExistingCaseModel
from ert_gui.models.mixins import BooleanModelMixin, BasicModelMixin


class IsCaseInitializedModel(ErtConnector, BooleanModelMixin, BasicModelMixin):

    def __init__(self):
        CaseSelectorModel().observable().attach(CaseSelectorModel.CURRENT_CHOICE_CHANGED_EVENT, self.__caseChanged)
        EnsembleSizeModel().observable().attach(EnsembleSizeModel.SPINNER_VALUE_CHANGED_EVENT, self.__caseChanged)
        InitializeFromScratchModel().observable().attach(InitializeFromScratchModel.BUTTON_TRIGGERED_EVENT, self.__caseChanged)
        InitializeFromExistingCaseModel().observable().attach(InitializeFromScratchModel.BUTTON_TRIGGERED_EVENT, self.__caseChanged)

        super(IsCaseInitializedModel, self).__init__()

    def __caseChanged(self):
        self.observable().notify(IsCaseInitializedModel.BOOLEAN_VALUE_CHANGED_EVENT)
        self.observable().notify(IsCaseInitializedModel.VALUE_CHANGED_EVENT)

    def getValue(self):
        """
         A message saying if the case is not initialized. Returns an empty string if the case is initialized.
         @rtype: str
        """
        if not self.isTrue():
            return "The current case is not initialized!"

        return ""

    def isTrue(self):
        """
        True if the current case is initialized!
        @rtype: bool
        """
        return self.ert().getEnkfFsManager().isInitialized()


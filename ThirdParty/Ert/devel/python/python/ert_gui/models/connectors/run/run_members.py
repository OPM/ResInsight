from ert_gui.models import ErtConnector
from ert_gui.models.connectors.ensemble_resizer import EnsembleSizeModel
from ert_gui.models.mixins import SelectableListModelMixin
from ert.util import StringList


class RunMembersModel(ErtConnector, SelectableListModelMixin):

    def __init__(self):
        EnsembleSizeModel().observable().attach(EnsembleSizeModel.SPINNER_VALUE_CHANGED_EVENT, self.__ensembleSizeChanged)
        super(RunMembersModel, self).__init__()

    def __ensembleSizeChanged(self):
        self.observable().notify(SelectableListModelMixin.LIST_CHANGED_EVENT)
        self.observable().notify(SelectableListModelMixin.SELECTION_CHANGED_EVENT)

    def getList(self):
        """ @rtype: StringList """
        size = EnsembleSizeModel().getSpinnerValue()
        return [str(num) for num in range(0, size)]

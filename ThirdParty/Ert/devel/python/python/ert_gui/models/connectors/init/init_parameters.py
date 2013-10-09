from ert.enkf.enums import EnkfVarType
from ert_gui.models import ErtConnector
from ert_gui.models.mixins import SelectableListModelMixin
from ert.util import StringList


class InitializationParametersModel(ErtConnector, SelectableListModelMixin):

    def getList(self):
        """ @rtype: StringList """
        return self.ert().ensembleConfig().getKeylistFromVarType(EnkfVarType.PARAMETER)



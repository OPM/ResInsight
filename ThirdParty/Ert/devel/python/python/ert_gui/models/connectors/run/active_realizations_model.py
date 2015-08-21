from ert.util import BoolVector
from ert_gui.models import ErtConnector
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.mixins import BasicModelMixin


class ActiveRealizationsModel(ErtConnector, BasicModelMixin):


    def __init__(self):
        self.__active_realizations = self.getDefaultValue()
        self.__custom = False
        EnsembleSizeModel().observable().attach(EnsembleSizeModel.SPINNER_VALUE_CHANGED_EVENT, self.__ensembleSizeChanged)
        super(ActiveRealizationsModel, self).__init__()


    def __ensembleSizeChanged(self):
        if not self.__custom:
            self.__active_realizations = self.getDefaultValue()
            self.observable().notify(self.VALUE_CHANGED_EVENT)

    def getValue(self):
        """ @rtype: str """
        return self.__active_realizations

    def setValue(self, active_realizations):
        if active_realizations is None or active_realizations.strip() == "" or active_realizations == self.getDefaultValue():
            self.__custom = False
            self.__active_realizations = self.getDefaultValue()
        else:
            self.__custom = True
            self.__active_realizations = active_realizations

        self.observable().notify(self.VALUE_CHANGED_EVENT)


    def getDefaultValue(self):
        size = EnsembleSizeModel().getValue()
        return "0-%d" % (size - 1)

    def getActiveRealizationsMask(self):
        count = EnsembleSizeModel().getValue()

        mask = BoolVector.createActiveMask(self.getValue())

        if mask is None:
            raise ValueError("Error while parsing range string!")

        if len(mask) > count:
            raise ValueError("Mask size changed %d != %d!" % (count, len(mask)))

        return mask













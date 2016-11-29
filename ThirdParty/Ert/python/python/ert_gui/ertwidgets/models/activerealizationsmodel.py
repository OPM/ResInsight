from ert.util import BoolVector
from ert_gui.ertwidgets.models.valuemodel import ValueModel
from ert_gui.ertwidgets.models.ertmodel import getRealizationCount


class ActiveRealizationsModel(ValueModel):
    def __init__(self):
        ValueModel.__init__(self, self.getDefaultValue())
        self._custom = False

    def setValue(self, active_realizations):
        if active_realizations is None or active_realizations.strip() == "" or active_realizations == self.getDefaultValue():
            self._custom = False
            ValueModel.setValue(self, self.getDefaultValue())
        else:
            self._custom = True
            ValueModel.setValue(self, active_realizations)

    def getDefaultValue(self):
        size = getRealizationCount()
        return "0-%d" % (size - 1)

    def getActiveRealizationsMask(self):
        count = getRealizationCount()

        mask = BoolVector.createActiveMask(self.getValue())

        if mask is None:
            raise ValueError("Error while parsing range string!")

        if len(mask) > count:
            raise ValueError("Mask size changed %d != %d!" % (count, len(mask)))

        return mask

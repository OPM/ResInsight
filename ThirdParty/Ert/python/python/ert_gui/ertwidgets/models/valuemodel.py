from PyQt4.QtCore import QObject, pyqtSignal, pyqtSlot

class ValueModel(QObject):
    valueChanged = pyqtSignal(str)

    def __init__(self, value=""):
        super(ValueModel, self).__init__()
        self._value = value

    def getValue(self):
        """ @rtype: str """
        return self._value

    @pyqtSlot(str)
    def setValue(self, value):
        self._value = value
        self.valueChanged.emit(value)

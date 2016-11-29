from PyQt4.QtCore import QObject, pyqtSignal


class SelectableListModel(QObject):
    modelChanged = pyqtSignal()
    selectionChanged = pyqtSignal()

    def __init__(self, items):
        QObject.__init__(self)
        self._selection = {}
        self._items = items

    def getList(self):
        return self._items

    def isValueSelected(self, value):
        return self._selection.get(value, True)

    def selectValue(self, value):
        self._setSelectState(value, True)
        self.selectionChanged.emit()

    def unselectValue(self, value):
        self._setSelectState(value, False)
        self.selectionChanged.emit()

    def unselectAll(self):
        for item in self.getList():
            self._setSelectState(item, False)

        self.selectionChanged.emit()

    def selectAll(self):
        for item in self.getList():
            self._setSelectState(item, True)

        self.selectionChanged.emit()

    def getSelectedItems(self):
        return [item for item in self.getList() if self.isValueSelected(item)]

    def _setSelectState(self, key, state):
        self._selection[key] = state

from PyQt4.QtCore import QAbstractItemModel, QModelIndex, Qt, QVariant
from PyQt4.QtGui import QColor

from ert_gui.ertwidgets import resourceIcon


class DataTypeKeysListModel(QAbstractItemModel):
    DEFAULT_DATA_TYPE = QColor(255, 255, 255)
    HAS_OBSERVATIONS = QColor(237, 218, 116)
    GROUP_ITEM = QColor(64, 64, 64)

    def __init__(self, ert):
        """
        @type ert: ert.enkf.EnKFMain
        """
        QAbstractItemModel.__init__(self)
        self.__ert = ert
        self.__icon = resourceIcon("ide/small/bullet_star")

    def keyManager(self):
        return self.__ert.getKeyManager()

    def index(self, row, column, parent=None, *args, **kwargs):
        return self.createIndex(row, column, parent)

    def parent(self, index=None):
        return QModelIndex()

    def rowCount(self, parent=None, *args, **kwargs):
        return len(self.keyManager().allDataTypeKeys())

    def columnCount(self, QModelIndex_parent=None, *args, **kwargs):
        return 1

    def data(self, index, role=None):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            items = self.keyManager().allDataTypeKeys()
            row = index.row()
            item = items[row]

            if role == Qt.DisplayRole:
                return item
            elif role == Qt.BackgroundRole:
                if self.keyManager().isKeyWithObservations(item):
                    return self.HAS_OBSERVATIONS

        return QVariant()

    def itemAt(self, index):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            row = index.row()
            return self.keyManager().allDataTypeKeys()[row]

        return None


    def isSummaryKey(self, key):
        return self.keyManager().isSummaryKey(key)

    def isBlockKey(self, key):
        return False

    def isGenKWKey(self, key):
        return self.keyManager().isGenKwKey(key)

    def isGenDataKey(self, key):
        return self.keyManager().isGenDataKey(key)

    def isCustomKwKey(self, key):
        return self.keyManager().isCustomKwKey(key)

    def isCustomPcaKey(self, key):
        return False

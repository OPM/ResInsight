from PyQt4.QtCore import QAbstractItemModel, QModelIndex, Qt, QVariant
from PyQt4.QtGui import QColor
from ert_gui.models.connectors.plot import DataTypeKeysModel
from ert_gui.widgets import util


class DataTypeKeysListModel(QAbstractItemModel):
    DEFAULT_DATA_TYPE = QColor(255, 255, 255)
    HAS_OBSERVATIONS = QColor(237, 218, 116)
    GROUP_ITEM = QColor(64, 64, 64)

    def __init__(self):
        QAbstractItemModel.__init__(self)
        self.__icon = util.resourceIcon("ide/small/bullet_star")
        self.__items = DataTypeKeysModel().getAllKeys()


    def index(self, row, column, parent=None, *args, **kwargs):
        return self.createIndex(row, column, parent)

    def parent(self, index=None):
        return QModelIndex()

    def rowCount(self, parent=None, *args, **kwargs):
        return len(self.__items)

    def columnCount(self, QModelIndex_parent=None, *args, **kwargs):
        return 1

    def data(self, index, role=None):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            items = self.__items
            row = index.row()
            item = items[row]

            if role == Qt.DisplayRole:
                return item
            elif role == Qt.BackgroundRole:
                if DataTypeKeysModel().isObservationKey(item):
                    return self.HAS_OBSERVATIONS

        return QVariant()

    def itemAt(self, index):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            row = index.row()
            return self.__items[row]

        return None


    def isSummaryKey(self, key):
        return DataTypeKeysModel().isSummaryKey(str(key))

    def isBlockKey(self, key):
        return DataTypeKeysModel().isBlockKey(str(key))

    def isGenKWKey(self, key):
        return DataTypeKeysModel().isGenKWKey(str(key))

    def isGenDataKey(self, key):
        return DataTypeKeysModel().isGenDataKey(str(key))

    def isCustomPcaKey(self, key):
        return DataTypeKeysModel().isCustomPcaKey(str(key))







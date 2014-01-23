from PyQt4.QtCore import QAbstractItemModel, QModelIndex, Qt, QVariant
from ert_gui.models.connectors.init import CaseList


class PlotCaseModel(QAbstractItemModel):

    def __init__(self):
        QAbstractItemModel.__init__(self)
        CaseList().observable().attach(CaseList.LIST_CHANGED_EVENT, self.updateItems)
        self.__data = None
        self.destroyed.connect(self.cleanup)


    def index(self, row, column, parent=None, *args, **kwargs):
        return self.createIndex(row, column, parent)

    def parent(self, index=None):
        return QModelIndex()

    def rowCount(self, parent=None, *args, **kwargs):
        items = self.getAllItems()
        return len(items)

    def columnCount(self, QModelIndex_parent=None, *args, **kwargs):
        return 1


    def data(self, index, role=None):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            items = self.getAllItems()
            row = index.row()
            item = items[row]

            if role == Qt.DisplayRole:
                return item

        return QVariant()

    def itemAt(self, index):
        assert isinstance(index, QModelIndex)

        if index.isValid():
            row = index.row()
            return self.getAllItems()[row]

        return None


    def getAllItems(self):
        if self.__data is None:
            self.updateItems()

        return self.__data

    def updateItems(self):
        self.beginResetModel()
        self.__data = CaseList().getAllCasesWithData()
        self.endResetModel()

    def cleanup(self):
        CaseList().observable().detach(CaseList.LIST_CHANGED_EVENT, self.updateItems)


    def __iter__(self):
        cur = 0
        while cur < self.rowCount():
            yield self.itemAt(self.index(cur, 0))
            cur += 1











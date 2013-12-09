from PyQt4.QtCore import QAbstractItemModel, QModelIndex, QVariant, Qt
from ert_gui.ide.wizards import TreeItem


class TreeModel(QAbstractItemModel):

    def __init__(self, tree_root, parent=None):
        QAbstractItemModel.__init__(self, parent)

        self.__root = tree_root

    def data(self, q_model_index, role=Qt.DisplayRole):

        item = self.item(q_model_index)

        if item is not None:
            if role == Qt.DisplayRole:
                return item.name()

        return QVariant()

    def item(self, q_model_index):
        if not q_model_index.isValid():
            return None

        return q_model_index.internalPointer()


    def flags(self, q_model_index):
        if not q_model_index.isValid():
            return 0

        item = self.item(q_model_index)

        if item.data() is None:
            return Qt.ItemIsEnabled
        else:
            return Qt.ItemIsEnabled | Qt.ItemIsSelectable


    def headerData(self, section, orientation, role=Qt.DisplayRole):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return self.__root.name()

        return QVariant()


    def index(self, row, column, parent=None):
        if parent is None:
            parent = QModelIndex()

        if not self.hasIndex(row, column, parent):
            return QModelIndex()

        if not parent.isValid():
            parent_item = self.__root
        else:
            parent_item = parent.internalPointer()

        child_item = parent_item.child(row)

        return self.createIndex(row, column, child_item)


    def parent(self, q_model_index):
        if not q_model_index.isValid():
            return QModelIndex()

        child_item = q_model_index.internalPointer()
        parent_item = child_item.parent()

        if parent_item == self.__root:
            return QModelIndex()

        return self.createIndex(parent_item.row(), 0, parent_item)

    def rowCount(self, parent):
        if parent is None:
            parent = QModelIndex()

        if parent.column() > 0:
            return 0

        if not parent.isValid():
            return len(self.__root)
        else:
            return len(parent.internalPointer())



    def columnCount(self, parent):
        return 1


    def emitChange(self):
        self.modelReset.emit()


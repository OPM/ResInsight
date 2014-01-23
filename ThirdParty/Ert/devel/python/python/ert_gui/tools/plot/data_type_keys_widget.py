from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QListView, QColor
from ert_gui.tools.plot import DataTypeKeysListModel
from ert_gui.widgets.legend import Legend


class DataTypeKeysWidget(QWidget):
    dataTypeKeySelected = pyqtSignal(str)

    def __init__(self):
        QWidget.__init__(self)

        layout = QVBoxLayout()

        self.model = DataTypeKeysListModel()
        self.data_type_keys_widget = QListView()
        self.data_type_keys_widget.setModel(self.model)
        self.data_type_keys_widget.selectionModel().selectionChanged.connect(self.itemSelected)

        layout.addWidget(self.data_type_keys_widget, 2)

        layout.addWidget(Legend("Default types", DataTypeKeysListModel.DEFAULT_DATA_TYPE))
        layout.addWidget(Legend("Observations available", DataTypeKeysListModel.HAS_OBSERVATIONS))

        self.setLayout(layout)


    def itemSelected(self):
        self.dataTypeKeySelected.emit(self.getSelectedItem())


    def getSelectedItem(self):
        """ @rtype: str """
        index = self.data_type_keys_widget.currentIndex()
        item = self.model.itemAt(index)
        return item

    def selectDefault(self):
        self.data_type_keys_widget.setCurrentIndex(self.model.index(0, 0))




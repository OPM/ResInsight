from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QListView, QHBoxLayout, QToolButton
from ert_gui.tools.plot import DataTypeKeysListModel, DataTypeProxyModel, FilterPopup
from ert_gui.widgets import util
from ert_gui.widgets.legend import Legend
from ert_gui.widgets.search_box import SearchBox


class DataTypeKeysWidget(QWidget):
    dataTypeKeySelected = pyqtSignal()

    def __init__(self, model):
        QWidget.__init__(self)

        self.__filter_popup = FilterPopup(self)
        self.__filter_popup.filterSettingsChanged.connect(self.onItemChanged)

        layout = QVBoxLayout()

        self.model = model
        self.filter_model = DataTypeProxyModel(self.model)

        filter_layout = QHBoxLayout()

        self.search_box = SearchBox()
        self.search_box.filterChanged.connect(self.setSearchString)
        filter_layout.addWidget(self.search_box)

        filter_popup_button = QToolButton()
        filter_popup_button.setIcon(util.resourceIcon("ide/cog_edit.png"))
        filter_popup_button.clicked.connect(self.showFilterPopup)
        filter_layout.addWidget(filter_popup_button)
        layout.addLayout(filter_layout)

        self.data_type_keys_widget = QListView()
        self.data_type_keys_widget.setModel(self.filter_model)
        self.data_type_keys_widget.selectionModel().selectionChanged.connect(self.itemSelected)

        layout.addSpacing(15)
        layout.addWidget(self.data_type_keys_widget, 2)
        layout.addStretch()

        # layout.addWidget(Legend("Default types", DataTypeKeysListModel.DEFAULT_DATA_TYPE))
        layout.addWidget(Legend("Observations available", DataTypeKeysListModel.HAS_OBSERVATIONS))

        self.setLayout(layout)

    def onItemChanged(self, item):
        # self.filter_model.setShowBlockKeys(item["block"])
        self.filter_model.setShowSummaryKeys(item["summary"])
        self.filter_model.setShowGenKWKeys(item["gen_kw"])
        self.filter_model.setShowGenDataKeys(item["gen_data"])
        self.filter_model.setShowCustomKwKeys(item["custom_kw"])
        # self.filter_model.setShowCustomPcaKeys(item["custom_pca"])


    def itemSelected(self):
        selected_item = self.getSelectedItem()
        if selected_item is not None:
            self.dataTypeKeySelected.emit()


    def getSelectedItem(self):
        """ @rtype: str """
        index = self.data_type_keys_widget.currentIndex()
        source_index = self.filter_model.mapToSource(index)
        item = self.model.itemAt(source_index)
        return item

    def selectDefault(self):
        self.data_type_keys_widget.setCurrentIndex(self.filter_model.index(0, 0))


    def setSearchString(self, filter):
        self.filter_model.setFilterFixedString(filter)

    def showFilterPopup(self):
        self.__filter_popup.show()


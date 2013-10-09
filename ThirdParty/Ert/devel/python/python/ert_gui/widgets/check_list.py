from PyQt4.QtCore import QSize, Qt, SIGNAL
from PyQt4.QtGui import QToolButton, QHBoxLayout, QLabel, QListWidget, QWidget, QVBoxLayout, QListWidgetItem, QMenu, QAction, QAbstractItemView, QListView
from ert_gui.models.mixins import ListModelMixin, SelectableModelMixin
from ert_gui.widgets.search_box import SearchBox
from ert_gui.widgets.util import resourceIcon
from ert_gui.widgets.helped_widget import HelpedWidget


class CheckList(HelpedWidget):
    
    def __init__(self, model, label="", help_link=""):
        HelpedWidget.__init__(self, "", help_link)

        layout = QVBoxLayout()

        widget = QWidget()
        widget.setLayout(layout)

        self.checkAllButton = QToolButton()
        self.checkAllButton.setIcon(resourceIcon("checked"))
        self.checkAllButton.setIconSize(QSize(16, 16))
        self.checkAllButton.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.checkAllButton.setAutoRaise(True)
        self.checkAllButton.setToolTip("Select all")

        self.uncheckAllButton = QToolButton()
        self.uncheckAllButton.setIcon(resourceIcon("notchecked"))
        self.uncheckAllButton.setIconSize(QSize(16, 16))
        self.uncheckAllButton.setToolButtonStyle(Qt.ToolButtonIconOnly)
        self.uncheckAllButton.setAutoRaise(True)
        self.uncheckAllButton.setToolTip("Unselect all")

        self.list = QListWidget()
        self.list.setContextMenuPolicy(Qt.CustomContextMenu)
        self.list.setSelectionMode(QAbstractItemView.ExtendedSelection)

        self.search_box = SearchBox()

        check_button_layout = QHBoxLayout()

        check_button_layout.setMargin(0)
        check_button_layout.setSpacing(0)
        check_button_layout.addWidget(QLabel(label))
        check_button_layout.addStretch(1)
        check_button_layout.addWidget(self.checkAllButton)
        check_button_layout.addWidget(self.uncheckAllButton)

        layout.addLayout(check_button_layout)
        layout.addWidget(self.list)
        layout.addWidget(self.search_box)

        self.addWidget(widget)

        self.connect(self.checkAllButton, SIGNAL('clicked()'), self.checkAll)
        self.connect(self.uncheckAllButton, SIGNAL('clicked()'), self.uncheckAll)
        self.connect(self.list, SIGNAL('itemChanged(QListWidgetItem*)'), self.itemChanged)
        self.search_box.filterChanged.connect(self.filterList)
        # self.connect(self.search_box, SIGNAL('filterChanged(str)'), self.filterList)

        self.connect(self.list, SIGNAL('customContextMenuRequested(QPoint)'), self.showContextMenu)

        assert isinstance(model, (SelectableModelMixin, ListModelMixin))
        self.model = model
        self.model.observable().attach(SelectableModelMixin.SELECTION_CHANGED_EVENT, self.modelChanged)
        self.model.observable().attach(ListModelMixin.LIST_CHANGED_EVENT, self.modelChanged)
        self.modelChanged()

    def itemChanged(self, item):
        """@type item: QListWidgetItem"""
        if item.checkState() == Qt.Checked:
            self.model.selectValue(str(item.text()))
        elif item.checkState() == Qt.Unchecked:
            self.model.unselectValue(str(item.text()))
        else:
            raise AssertionError("Unhandled checkstate!")

    def modelChanged(self):
        self.list.clear()

        items = self.model.getList()

        for item in items:
            list_item = QListWidgetItem(item)
            list_item.setFlags(list_item.flags() | Qt.ItemIsUserCheckable)

            if self.model.isValueSelected(item):
                list_item.setCheckState(Qt.Checked)
            else:
                list_item.setCheckState(Qt.Unchecked)

            self.list.addItem(list_item)

        self.filterList(self.search_box.filter())

    def setSelectionEnabled(self, enabled):
        self.setEnabled(enabled)
        self.checkAllButton.setEnabled(enabled)
        self.uncheckAllButton.setEnabled(enabled)

    def filterList(self, filter):
        filter = filter.lower()

        for index in range(0, self.list.count()):
            item = self.list.item(index)
            text = str(item.text()).lower()

            if filter == "":
                item.setHidden(False)
            elif filter in text:
                item.setHidden(False)
            else:
                item.setHidden(True)


    def checkAll(self):
        self.model.selectAll()

    def uncheckAll(self):
        self.model.unselectAll()

    def checkSelected(self):
        items = []
        for item in self.list.selectedItems():
            items.append(str(item.text()))

        for item in items:
            self.model.selectValue(item)

    def uncheckSelected(self):
        items = []
        for item in self.list.selectedItems():
            items.append(str(item.text()))

        for item in items:
            self.model.unselectValue(item)

    def showContextMenu(self, point):
        p = self.list.mapToGlobal(point)
        menu = QMenu()
        check_selected = menu.addAction("Check selected")
        uncheck_selected = menu.addAction("Uncheck selected")
        menu.addSeparator()
        clear_selection = menu.addAction("Clear selection")

        selected_item = menu.exec_(p)

        if selected_item == check_selected:
            self.checkSelected()
        elif selected_item == uncheck_selected:
            self.uncheckSelected()
        elif selected_item == clear_selection:
            self.list.clearSelection()


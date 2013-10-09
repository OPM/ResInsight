from PyQt4.QtCore import SIGNAL, Qt
from PyQt4.QtGui import QTableWidget, QAbstractItemView, QMessageBox, QTableWidgetItem, QInputDialog, QLineEdit
from ert_gui.models.mixins import DictionaryModelMixin
from ert_gui.widgets.add_remove_widget import AddRemoveWidget
from ert_gui.widgets.helped_widget import HelpedWidget


class KeywordTable(HelpedWidget):
    """Shows a table of key/value pairs. The data structure expected and sent to the getter and setter is a dictionary of values."""
    def __init__(self, model, table_label="", help_link=""):
        HelpedWidget.__init__(self, table_label, help_link)

        self.table = QTableWidget(self)
        self.table.setColumnCount(2)
        self.setColumnHeaders()
        self.table.verticalHeader().setHidden(True)
        self.table.setColumnWidth(0, 150)
        self.table.horizontalHeader().setStretchLastSection(True)

        self.table.setMinimumHeight(110)
        self.table.setSelectionMode(QAbstractItemView.SingleSelection)
        self.table.setSelectionBehavior(QAbstractItemView.SelectRows)

        self.addWidget(self.table)

        self.addWidget(AddRemoveWidget(self.addItem, self.removeItem))

        self.connect(self.table, SIGNAL('cellChanged(int,int)'), self.tableChanged)

        assert isinstance(model, DictionaryModelMixin)
        self.model = model
        model.observable().attach(DictionaryModelMixin.DICTIONARY_CHANGED_EVENT, self.modelChanged)
        self.modelChanged()


    def setColumnHeaders(self, keyword_name="Keyword", value_name="Value"):
        self.headers = [keyword_name, value_name]
        self.table.setHorizontalHeaderLabels(self.headers)

    def addItem(self):
        """Called by the add button to insert a new keyword"""
        title = "New %s" % self.headers[0]
        description = "Enter new %s:" % self.headers[0]
        (new_keyword, ok) = QInputDialog.getText(self, title, description, QLineEdit.Normal)

        if ok:
            new_keyword = str(new_keyword).strip()
            self.model.addKey(new_keyword)

    def removeItem(self):
        """Called by the remove button to remove a selected keyword"""
        current_row = self.table.currentRow()

        if current_row >= 0:
            do_delete = QMessageBox.question(self, "Delete row?", "Are you sure you want to delete the key/value pair?", QMessageBox.Yes | QMessageBox.No )

            if do_delete:
                key_item = self.table.item(current_row, 0)
                if key_item is not None:
                    key = str(key_item.text()).strip()
                    self.model.removeKey(key)


    def tableChanged(self, row, column):
        """Called whenever the contents of a cell changes."""
        key_item = self.table.item(row, 0)

        if key_item is not None:
            key = str(key_item.text()).strip()
            value_item = self.table.item(row, 1)

            if value_item is not None:
                value = str(value_item.text()).strip()

                self.model.setValueForKey(key, value)


    def modelChanged(self):
        """Retrieves data from the model and inserts it into the table."""
        values = self.model.getDictionary()

        blocked = self.table.blockSignals(True)

        for row in reversed(range(self.table.rowCount())):
            self.table.removeRow(row)

        row = 0
        for key in values:
            key_item = QTableWidgetItem(str(key))
            key_item.setFlags(key_item.flags() ^ Qt.ItemIsEditable)
            value_item = QTableWidgetItem(str(values[key]))
            self.table.insertRow(row)
            self.table.setItem(row, 0, key_item)
            self.table.setItem(row, 1, value_item)
            row += 1

        self.table.blockSignals(blocked)


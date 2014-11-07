from PyQt4.QtGui import QListWidget, QInputDialog, QLineEdit, QMessageBox, QAbstractItemView
from ert_gui.models.mixins import ListModelMixin
from ert_gui.widgets.add_remove_widget import AddRemoveWidget
from ert_gui.widgets.helped_widget import HelpedWidget


class KeywordList(HelpedWidget):
    """Shows a list of keywords. The data structure expected and sent to the getter and setter is an array of values."""
    def __init__(self, model, list_label="", help_link=""):
        HelpedWidget.__init__(self, list_label, help_link)

        assert isinstance(model, ListModelMixin)
        self.model = model
        self.keyword_list = []

        self.list = QListWidget(self)
        self.list.setMinimumHeight(100)
        self.list.setMaximumHeight(150)
        self.default_selection_mode = self.list.selectionMode()


        self.addWidget(self.list)

        self.addRemoveWidget = AddRemoveWidget(self.addItem, self.removeItem)
        self.addWidget(self.addRemoveWidget)

        self.title = "New keyword"
        self.description = "Enter name of keyword:"

        self.model.observable().attach(ListModelMixin.LIST_CHANGED_EVENT, self.modelChanged)

        self.modelChanged()

    def setSelectable(self, selectable):
        if selectable:
            self.list.setSelectionMode(self.default_selection_mode)
        else:
            self.list.setSelectionMode(QAbstractItemView.NoSelection)

    def setPopupLabels(self, title, description):
        """Change the labels of the default popup."""
        self.title = title
        self.description = description

    def newKeywordPopup(self, keyword_list):
        """
        Pops up a message box asking for a new keyword.
        Override this and return a string to customize the input dialog - Empty string equals canceled.
        The provided list are the already defined keywords
        """
        new_keyword, ok = QInputDialog.getText(self, self.tr(self.title), self.tr(self.description), QLineEdit.Normal)

        if ok:
            return str(new_keyword).strip()
        else:
            return ""

    def addItem(self):
        """Called by the add button to insert a new keyword"""
        new_keyword = self.newKeywordPopup(self.keyword_list)
        if not new_keyword == "":
            self.model.addItem(new_keyword)


    def removeItem(self):
        """Called by the remove button to remove a selected keyword"""
        if not self.list.currentItem() is None:
            row = self.list.currentRow()
            try:
                self.model.removeItem(self.keyword_list[row])
            except NotImplementedError:
                message = "Support for removal of items has not been implemented!"
                QMessageBox.information(self, "Not implemented!", message)


    def modelChanged(self):
        """Retrieves data from the model and inserts it into the list"""
        keywords = self.model.getList()

        self.list.clear()

        self.keyword_list = keywords

        for keyword in keywords:
            self.list.addItem(keyword)


    def cleanup(self):
        self.model.observable().detach(ListModelMixin.LIST_CHANGED_EVENT, self.modelChanged)


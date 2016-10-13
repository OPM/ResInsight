from PyQt4.QtCore import pyqtSignal, Qt
from PyQt4.QtGui import QLineEdit, QColor


class SearchBox(QLineEdit):
    passive_color = QColor(194, 194, 194)

    filterChanged = pyqtSignal(['PyQt_PyObject'])

    def __init__(self):
        QLineEdit.__init__(self)

        self.setToolTip("Type to search!")
        self.active_color = self.palette().color(self.foregroundRole())
        self.disable_search = True
        self.presentSearch()
        self.textChanged.connect(self.__emitFilterChanged)

    def __emitFilterChanged(self, filter):
        self.filterChanged.emit(self.filter())
        # self.emit(SIGNAL('filterChanged(PyQt_PyObject)'), self.filter())

    def filter(self):
        if self.disable_search:
            return ""
        else:
            return str(self.text())

    def presentSearch(self):
        """Is called to present the greyed out search"""
        self.disable_search = True
        self.setText("Search")
        palette = self.palette()
        palette.setColor(self.foregroundRole(), self.passive_color)
        self.setPalette(palette)

    def activateSearch(self):
        """Is called to remove the greyed out search"""
        self.disable_search = False
        self.setText("")
        palette = self.palette()
        palette.setColor(self.foregroundRole(), self.active_color)
        self.setPalette(palette)

    def enterSearch(self):
        """Called when the line edit gets the focus"""
        if str(self.text()) == "Search":
            self.activateSearch()

    def exitSearch(self):
        """Called when the line edit looses focus"""
        if str(self.text()) == "":
            self.presentSearch()

    def focusInEvent(self, focus_event):
        QLineEdit.focusInEvent(self, focus_event)
        self.enterSearch()

    def focusOutEvent(self, focus_event):
        QLineEdit.focusOutEvent(self, focus_event)
        self.exitSearch()


    def keyPressEvent(self, key_event):
        if key_event.key() == Qt.Key_Escape:
            self.clear()
            self.clearFocus()
        else:
            QLineEdit.keyPressEvent(self, key_event)
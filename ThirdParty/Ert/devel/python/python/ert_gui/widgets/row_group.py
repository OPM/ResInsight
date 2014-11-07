from PyQt4.QtGui import QHBoxLayout, QWidget
from ert_gui.widgets.helped_widget import HelpedWidget


class RowGroup(HelpedWidget):

    def __init__(self, label="", help_link=""):
        HelpedWidget.__init__(self, label, help_link)

        widget = QWidget()
        self.layout = QHBoxLayout()
        self.layout.setMargin(0)

        widget.setLayout(self.layout)

        HelpedWidget.addWidget(self, widget)


    def addWidget(self, widget):
        self.layout.addWidget(widget)

    def addGroupStretch(self):
        self.layout.addStretch(1)



from PyQt4.QtCore import Qt, SIGNAL, pyqtSlot, SLOT
from PyQt4.QtGui import QWidget, QDialog, QVBoxLayout, QLayout, QPushButton


class PopupDialog(QDialog):

    def __init__(self, title, panel, parent=None):
        QDialog.__init__(self, parent)

        self.__initialized = False
        self.__position = None
        self.__geometry = None
        self.__button = QPushButton("Button")
        self.__button.setCheckable(True)

        self.setWindowTitle(title)
        self.setModal(False)
        self.setWindowFlags(self.windowFlags() & ~Qt.WindowContextHelpButtonHint)


        layout = QVBoxLayout()
        layout.setMargin(0)
        layout.setSizeConstraint(QLayout.SetFixedSize) # not resizable!!!
        layout.addWidget(panel)
        self.setLayout(layout)

        self.connect(self, SIGNAL("accepted()"), self.closeDialog)
        self.connect(self, SIGNAL("rejected()"), self.closeDialog)

        self.connect(self.__button, SIGNAL("toggled(bool)"), self, SLOT("buttonToggle(bool)"))

    def showDialog(self):
        self.show()

        if self.__initialized:
            self.move(self.__position)
            self.setGeometry(self.__geometry)

        self.raise_()
        self.activateWindow()

        if not self.__button.isChecked():
            self.__button.setChecked(True)

    def closeDialog(self):
        self.__position = self.pos()
        self.__geometry = self.geometry()
        self.__initialized = True
        self.setVisible(False)

        if self.__button.isChecked():
            self.__button.setChecked(False)

    def getButton(self):
        return self.__button

    @pyqtSlot(bool)
    def buttonToggle(self, toggle):
        if toggle:
            self.showDialog()
        else:
            self.closeDialog()

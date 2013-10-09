from PyQt4.QtCore import QSize, SIGNAL
from PyQt4.QtGui import QWidget, QToolButton, QVBoxLayout, QHBoxLayout
from ert_gui.widgets.util import resourceIcon


class AddRemoveWidget(QWidget):
    """
    A simple class that provides to vertically positioned buttons for adding and removing something.
    The addFunction and removeFunction functions must be provided.
    """

    def __init__(self, addFunction=None, removeFunction=None, horizontal=False):
        QWidget.__init__(self)

        self.addButton = QToolButton(self)
        self.addButton.setIcon(resourceIcon("add"))
        self.addButton.setIconSize(QSize(16, 16))
        self.connect(self.addButton, SIGNAL('clicked()'), addFunction)

        self.removeButton = QToolButton(self)
        self.removeButton.setIcon(resourceIcon("remove"))
        self.removeButton.setIconSize(QSize(16, 16))
        self.connect(self.removeButton, SIGNAL('clicked()'), removeFunction)

        if horizontal:
            self.buttonLayout = QHBoxLayout()
        else:
            self.buttonLayout = QVBoxLayout()

        self.buttonLayout.setMargin(0)

        if horizontal:
            self.buttonLayout.addStretch(1)

        self.buttonLayout.addWidget(self.addButton)
        self.buttonLayout.addWidget(self.removeButton)

        if not horizontal:
            self.buttonLayout.addStretch(1)
        else:
            self.buttonLayout.addSpacing(2)

        self.setLayout(self.buttonLayout)

    def enableAddButton(self, state):
        """Enable or disable the add button"""
        self.addButton.setEnabled(state)

    def enableRemoveButton(self, state):
        """Enable or disable the remove button"""
        self.removeButton.setEnabled(state)
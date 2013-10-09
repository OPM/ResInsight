from PyQt4.QtCore import QSize
from PyQt4.QtGui import QWidget, QColor, QLabel, QHBoxLayout
from ert_gui.pages.message_center import MessageCenter

from ert_gui.widgets.util import resourceImage


class HelpedWidget(QWidget):
    """
    HelpedWidget is a class that enables embedded help messages in widgets.
    """

    STRONG_ERROR_COLOR = QColor(255, 215, 215)
    ERROR_COLOR = QColor(255, 235, 235)
    INVALID_COLOR = QColor(235, 235, 255)

    WARNING = "warning"
    EXCLAMATION = "exclamation"

    def __init__(self, widget_label="", help_link=""):
        QWidget.__init__(self)

        self.validation_label = QLabel()
        self.validation_label.setMaximumSize(QSize(16, 16))
        self.validation_label.setPixmap(resourceImage(HelpedWidget.WARNING))
        self.validation_label.setHidden(True)

        if not widget_label == "":
            self.label = widget_label + ":"
        else:
            self.label = ""

        self.help_link = help_link
        self.validation_message = None
        self.validation_type = None

        self.widget_layout = QHBoxLayout()
        self.widget_layout.setMargin(0)
        self.widget_layout.addWidget(self.validation_label)
        self.setLayout(self.widget_layout)


    def getLabel(self):
        """Returns the label of this widget if set or empty string."""
        return self.label

    def addLayout(self, layout):
        """Add a layout to the layout of this widget."""
        self.widget_layout.addLayout(layout)


    def addWidget(self, widget):
        """Add a widget to the layout of this widget."""
        self.widget_layout.addWidget(widget)

    def addStretch(self, stretch=1):
        """Add stretch between widgets. """
        self.widget_layout.addStretch(stretch)

    def setValidationMessage(self, message, validation_type=WARNING):
        """Add a warning or information icon to the widget with a tooltip"""
        if message == "":
            self.validation_type = None
            self.validation_message = None
            self.validation_label.setHidden(True)
            self.validation_label.setToolTip("")
        else:
            self.validation_type = validation_type
            self.validation_message = message
            self.validation_label.setHidden(False)
            self.validation_label.setToolTip(message)
            self.validation_label.setPixmap(resourceImage(validation_type))

        MessageCenter().setWarning(self, self.validation_message)

    def enterEvent(self, event):
        QWidget.enterEvent(self, event)
        MessageCenter().setHelpMessageLink(self.help_link)





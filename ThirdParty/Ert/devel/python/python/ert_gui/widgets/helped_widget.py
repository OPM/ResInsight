from PyQt4.QtCore import QSize, pyqtSignal
from PyQt4.QtGui import QWidget, QColor, QLabel, QHBoxLayout, QIcon, QPixmap
from ert_gui.pages.message_center import MessageCenter
from ert_gui.widgets.error_popup import ErrorPopup

from ert_gui.widgets.util import resourceImage



class HelpedWidget(QWidget):
    """
    HelpedWidget is a class that enables embedded help messages in widgets.
    """

    STRONG_ERROR_COLOR = QColor(255, 215, 215)
    ERROR_COLOR = QColor(255, 235, 235)
    INVALID_COLOR = QColor(235, 235, 255)

    WARNING = "warning"
    EXCLAMATION = "ide/small/exclamation"

    validationChanged = pyqtSignal(bool)

    def __init__(self, widget_label="", help_link=""):
        QWidget.__init__(self)

        self.validation_label = QLabel()
        self.validation_label.setParent(self)
        self.validation_label.setMinimumSize(QSize(16, 16))
        self.validation_label.setMaximumSize(QSize(16, 16))

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
        self.setMinimumHeight(20)

        HelpedWidget.__error_popup = ErrorPopup()
        self.destroyed.connect(self.cleanup)


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
        message = message.strip()
        if message == "":
            self.validation_type = None
            self.validation_message = None
            self.validation_label.setPixmap(QPixmap())
            HelpedWidget.__error_popup.hide()
            self.validationChanged.emit(True)

        else:
            self.validation_type = validation_type
            self.validation_message = message
            self.validation_label.setPixmap(resourceImage(validation_type))
            HelpedWidget.__error_popup.presentError(self, self.validation_message)
            self.validationChanged.emit(False)

        MessageCenter().setWarning(self, self.validation_message)


    def isValid(self):
        return self.validation_message is None


    def hideValidationLabel(self):
        self.validation_label.setHidden(True)

    def enterEvent(self, event):
        QWidget.enterEvent(self, event)
        MessageCenter().setHelpMessageLink(self.help_link)

        # if HelpedWidget.__error_popup is None:
        #     HelpedWidget.__error_popup = ErrorPopup()

        if self.validation_message is not None:
            HelpedWidget.__error_popup.presentError(self, self.validation_message)


    def leaveEvent(self, event):
        QWidget.leaveEvent(self, event)

        if HelpedWidget.__error_popup is not None:
            HelpedWidget.__error_popup.hide()


    def cleanup(self):
        """ Remove any model attachment or similar. Called when QT object is destroyed."""
        pass




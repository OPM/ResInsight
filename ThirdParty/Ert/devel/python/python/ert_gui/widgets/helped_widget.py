from PyQt4.QtCore import QSize, pyqtSignal
from PyQt4.QtGui import QWidget, QColor, QLabel, QHBoxLayout, QIcon, QPixmap
from ert_gui.tools import HelpCenter
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

        if not widget_label == "":
            self.label = widget_label + ":"
        else:
            self.label = ""

        self.help_link = help_link
        self.validation_message = None
        self.validation_type = None

        self.widget_layout = QHBoxLayout()
        self.widget_layout.setMargin(0)
        self.setLayout(self.widget_layout)
        self.setMinimumHeight(20)

        self.__error_popup = ErrorPopup()
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
            self.__error_popup.hide()
            self.validationChanged.emit(True)

        else:
            self.validation_type = validation_type
            self.validation_message = message
            if self.hasFocus() or self.underMouse():
                self.__error_popup.presentError(self, self.validation_message)
            self.validationChanged.emit(False)

        # HelpCenter.getHelpCenter("ERT").setHelpMessageLink()
        # MessageCenter().setWarning(self, self.validation_message)


    def isValid(self):
        return self.validation_message is None

    def includeLabel(self):
        label_widget = QLabel(self.label)
        self.widget_layout.insertWidget(0, label_widget)


    def enterEvent(self, event):
        QWidget.enterEvent(self, event)
        try:
            HelpCenter.getHelpCenter("ERT").setHelpMessageLink(self.help_link)
        except AttributeError:
            pass

        # if HelpedWidget.__error_popup is None:
        #     HelpedWidget.__error_popup = ErrorPopup()

        if self.validation_message is not None:
            self.__error_popup.presentError(self, self.validation_message)


    def leaveEvent(self, event):
        QWidget.leaveEvent(self, event)

        if self.__error_popup is not None:
            self.__error_popup.hide()


    def cleanup(self):
        """ Remove any model attachment or similar. Called when QT object is destroyed."""
        pass

    def hideEvent(self, hide_event):
        self.__error_popup.hide()
        super(HelpedWidget, self).hideEvent(hide_event)


    @staticmethod
    def addHelpToWidget(widget, link):
        original_enter_event = widget.enterEvent

        def enterEvent(event):
            original_enter_event(event)
            try:
                HelpCenter.getHelpCenter("ERT").setHelpMessageLink(link)
            except AttributeError:
                pass

        widget.enterEvent = enterEvent


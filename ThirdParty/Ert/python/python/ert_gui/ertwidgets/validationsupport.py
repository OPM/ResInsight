from PyQt4.QtCore import Qt, QPoint, QObject, pyqtSignal

from PyQt4.QtGui import QWidget, QVBoxLayout, QSizePolicy, QFrame, QColor, QLabel


class ErrorPopup(QWidget):
    error_template = ("<html>"
                      "<table style='background-color: #ffdfdf;'width='100%%'>"
                      "<tr><td style='font-weight: bold; padding-left: 5px;'>Warning:</td></tr>"
                      "%s"
                      "</table>"
                      "</html>")

    def __init__(self):
        QWidget.__init__(self, None, Qt.ToolTip)
        self.resize(300, 50)

        self.setContentsMargins(0, 0, 0, 0)
        layout = QVBoxLayout()
        layout.setMargin(0)

        self._error_widget = QLabel("")
        self._error_widget.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Minimum)
        self._error_widget.setFrameStyle(QFrame.Box)
        self._error_widget.setWordWrap(True)
        self._error_widget.setScaledContents(True)
        # self.warning_widget.setAlignment(Qt.AlignHCenter)
        self._error_widget.setTextFormat(Qt.RichText)
        layout.addWidget(self._error_widget)

        self.setLayout(layout)

    def presentError(self, widget, error):
        assert isinstance(widget, QWidget)

        self._error_widget.setText(ErrorPopup.error_template % error)
        self.show()

        size_hint = self.sizeHint()
        rect = widget.rect()
        p = widget.mapToGlobal(QPoint(rect.left(), rect.top()))

        self.setGeometry(p.x(), p.y() - size_hint.height() - 5, size_hint.width(), size_hint.height())

        self.raise_()


class ValidationSupport(QObject):
    STRONG_ERROR_COLOR = QColor(255, 215, 215)
    ERROR_COLOR = QColor(255, 235, 235)
    INVALID_COLOR = QColor(235, 235, 255)

    WARNING = "warning"
    EXCLAMATION = "ide/small/exclamation"

    validationChanged = pyqtSignal(bool)

    def __init__(self, validation_target):
        """ @type validation_target: QWidget """
        QObject.__init__(self)

        self._validation_target = validation_target
        self._validation_message = None
        self._validation_type = None
        self._error_popup = ErrorPopup()

        self._originalEnterEvent = validation_target.enterEvent
        self._originalLeaveEvent = validation_target.leaveEvent
        self._originalHideEvent = validation_target.hideEvent

        def enterEvent(event):
            self._originalEnterEvent(event)

            if not self.isValid():
                self._error_popup.presentError(self._validation_target, self._validation_message)

        validation_target.enterEvent = enterEvent

        def leaveEvent(event):
            self._originalLeaveEvent(event)

            if self._error_popup is not None:
                self._error_popup.hide()

        validation_target.leaveEvent = leaveEvent

        def hideEvent(hide_event):
            self._error_popup.hide()
            self._originalHideEvent(hide_event)

        validation_target.hideEvent = hideEvent

    def setValidationMessage(self, message, validation_type=WARNING):
        """Add a warning or information icon to the widget with a tooltip"""
        message = message.strip()
        if message == "":
            self._validation_type = None
            self._validation_message = None
            self._error_popup.hide()
            self.validationChanged.emit(True)

        else:
            self._validation_type = validation_type
            self._validation_message = message
            if self._validation_target.hasFocus() or self._validation_target.underMouse():
                self._error_popup.presentError(self._validation_target, self._validation_message)
            self.validationChanged.emit(False)

    def isValid(self):
        return self._validation_message is None

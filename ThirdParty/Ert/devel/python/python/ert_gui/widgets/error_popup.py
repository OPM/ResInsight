from PyQt4.QtCore import Qt, QPoint
from PyQt4.QtGui import QWidget, QVBoxLayout, QLabel, QFrame, QSizePolicy


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

        self.error_widget = QLabel("")
        self.error_widget.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Minimum)
        self.error_widget.setFrameStyle(QFrame.Box)
        self.error_widget.setWordWrap(True)
        self.error_widget.setScaledContents(True)
        # self.warning_widget.setAlignment(Qt.AlignHCenter)
        self.error_widget.setTextFormat(Qt.RichText)
        layout.addWidget(self.error_widget)

        self.setLayout(layout)

    def presentError(self, widget, error):
        assert isinstance(widget, QWidget)

        self.error_widget.setText(ErrorPopup.error_template % error)
        self.show()

        size_hint = self.sizeHint()
        rect = widget.rect()
        p = widget.mapToGlobal(QPoint(rect.left(), rect.top()))

        self.setGeometry(p.x(), p.y() - size_hint.height() - 5, size_hint.width(), size_hint.height())

        self.raise_()
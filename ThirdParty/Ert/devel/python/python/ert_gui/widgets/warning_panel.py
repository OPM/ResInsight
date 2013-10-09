from PyQt4.QtCore import Qt, pyqtSlot
from PyQt4.QtGui import QLabel
from ert_gui.pages.message_center import MessageCenter
from ert_gui.widgets.helped_widget import HelpedWidget


class WarningPanel(HelpedWidget):

    warning_template = ("<html>"
                        "<table style='background-color: #ffdfdf;'width='100%%'>"
                        "<tr><td style='font-weight: bold; padding-left: 5px;'>Warning:</td></tr>"
                        "%s"
                        "</table>"
                        "</html>")

    warning_row_template = "<tr><td style='padding: 5px;'>%s</td></tr>"

    def __init__(self, label="", help_link=""):
        HelpedWidget.__init__(self, label, help_link)

        self.warning_widget = QLabel("")
        self.warning_widget.setWordWrap(True)
        self.warning_widget.setScaledContents(True)
        self.warning_widget.setAlignment(Qt.AlignHCenter)
        self.warning_widget.setTextFormat(Qt.RichText)
        self.warning_widget.setVisible(False)

        self.addWidget(self.warning_widget)
        self.warnings = {}

        MessageCenter().addWarningMessageListeners(self)


    def setWarning(self, reference, warning):
        if warning is None:
            warning = ""

        warning = warning.strip()

        if (warning == "" or warning is None) and reference in self.warnings:
            self.warnings.pop(reference)

        if not warning == "":
            self.warnings[reference] = warning

        if len(self.warnings) > 0:
            message = ""
            for key in self.warnings:
                message += self.warning_row_template % self.warnings[key]

            self.warning_widget.setText(self.warning_template % message)
        else:
            self.warning_widget.setText("")

        self.warning_widget.setHidden(len(self.warnings) == 0)
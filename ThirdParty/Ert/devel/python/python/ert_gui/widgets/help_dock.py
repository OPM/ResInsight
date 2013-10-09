from PyQt4.QtCore import Qt, SIGNAL
from PyQt4.QtGui import QDockWidget, QLabel, QWidget, QVBoxLayout

import os
from ert_gui.pages.message_center import MessageCenter


class HelpDock(QDockWidget):
    __instance = None
    help_prefix = None
    default_help_string = "No help available!"
    validation_template = ("<html>"
                           "<table style='background-color: #ffefef;'width='100%%'>"
                           "<tr><td style='font-weight: bold; padding-left: 5px;'>Notice:</td></tr>"
                           "<tr><td style='padding: 5px;'>%s</td></tr>"
                           "</table>"
                           "</html>")

    def __init__(self):
        QDockWidget.__init__(self, "Help")
        self.setObjectName("HelpDock")

        widget = QWidget()
        widget.setStyleSheet("background-color: #ffffe0")
        layout = QVBoxLayout()
        widget.setLayout(layout)

        self.link_widget = QLabel()
        self.link_widget.setStyleSheet("font-weight: bold")
        self.link_widget.setMinimumHeight(20)

        self.help_widget = QLabel(HelpDock.default_help_string)
        self.help_widget.setWordWrap(True)
        self.help_widget.setTextFormat(Qt.RichText)

        self.validation_widget = QLabel("")
        self.validation_widget.setWordWrap(True)
        self.validation_widget.setScaledContents(True)
        self.validation_widget.setAlignment(Qt.AlignHCenter)
        self.validation_widget.setTextFormat(Qt.RichText)


        layout.addWidget(self.link_widget)
        layout.addWidget(self.help_widget)
        layout.addStretch(1)
        layout.addWidget(self.validation_widget)

        self.setWidget(widget)

        self.help_messages = {}

        MessageCenter().addHelpMessageListeners(self)

    def __new__(cls):
        if cls.__instance is None:
            cls.__instance = QDockWidget.__new__(cls)
            return cls.__instance
        return None


    @classmethod
    def getInstance(cls):
        """ @rtype: HelpDock """
        if cls.__instance is None:
            cls()

        return cls.__instance

    @classmethod
    def setHelpMessageLink(cls, help_link):
        instance = cls.getInstance()
        if not help_link in instance.help_messages:
            help_message = HelpDock.resolveHelpLink(help_link)
            if help_message is not None:
                instance.help_messages[help_link] = help_message
            else:
                instance.help_messages[help_link] = instance.default_help_string

        instance.link_widget.setText(help_link)
        instance.help_widget.setText(instance.help_messages[help_link])
        instance.validation_widget.setText("")

    @classmethod
    def setValidationMessage(cls, message):
        instance = cls.getInstance()
        instance.validation_widget.setHidden(message is None)
        instance.validation_widget.setText(instance.validation_template % message)


    # The setHelpLinkPrefix should be set to point to a directory
    # containing (directories) with html help files. In the current
    # implementation this variable is set from the gert_main.py script.
    @classmethod
    def setHelpLinkPrefix(cls, prefix):
        cls.help_prefix = prefix

    @classmethod
    def getTemplate(cls):
        path = cls.help_prefix + "template.html"
        if os.path.exists(path) and os.path.isfile(path):
            f = open(path, 'r')
            template = f.read()
            f.close()
            return template
        else:
            return "<html>%s</html>"

    @classmethod
    def resolveHelpLink(cls, help_link):
        """
        Reads a HTML file from the help directory.
        The HTML must follow the specification allowed by QT here: http://doc.trolltech.com/4.6/richtext-html-subset.html
        """

        # This code can be used to find widgets with empty help labels
        #    if label.strip() == "":
        #        raise AssertionError("NOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!")

        path = cls.help_prefix + help_link + ".html"
        if os.path.exists(path) and os.path.isfile(path):
            f = open(path, 'r')
            help = f.read()
            f.close()
            return cls.getTemplate() % help
        else:
        # This code automatically creates empty help files
        #        sys.stderr.write("Missing help file: '%s'\n" % label)
        #        if not label == "" and not label.find("/") == -1:
        #            sys.stderr.write("Creating help file: '%s'\n" % label)
        #            directory, filename = os.path.split(path)
        #
        #            if not os.path.exists(directory):
        #                os.makedirs(directory)
        #
        #            file_object = open(path, "w")
        #            file_object.write(label)
        #            file_object.close()
            return None


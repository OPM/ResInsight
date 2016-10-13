import re
import shutil

from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QToolBar, QMessageBox, QSizePolicy, QFileDialog

from ert_gui.ertwidgets import SearchBox, resourceIcon
from ert_gui.ide.highlighter import KeywordHighlighter
from ert_gui.ide.keywords.definitions.path_argument import PathArgument
from ert_gui.tools.ide import IdePanel


class ConfigurationPanel(QWidget):

    reloadApplication = pyqtSignal(str)

    def __init__(self, config_file_path, help_tool):
        QWidget.__init__(self)

        layout = QVBoxLayout()

        toolbar = QToolBar("toolbar")


        save_action = toolbar.addAction(resourceIcon("ide/disk"), "Save")
        save_action.triggered.connect(self.save)

        save_as_action = toolbar.addAction(resourceIcon("ide/save_as"), "Save As")
        save_as_action.triggered.connect(self.saveAs)

        # reload_icon = toolbar.style().standardIcon(QStyle.SP_BrowserReload)
        # reload_action = toolbar.addAction(reload_icon, "Reload")
        # reload_action.triggered.connect(self.reload)

        toolbar.addSeparator()

        toolbar.addAction(help_tool.getAction())


        stretchy_separator = QWidget()
        stretchy_separator.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        toolbar.addWidget(stretchy_separator)


        search = SearchBox()
        search.setMaximumWidth(200)
        search.setContentsMargins(5, 2, 5, 2)

        toolbar.addWidget(search)

        layout.addWidget(toolbar)

        self.ide_panel = IdePanel()
        layout.addWidget(self.ide_panel, 1)

        self.config_file_path = config_file_path

        with open(config_file_path) as f:
            config_file_text = f.read()

        self.highlighter = KeywordHighlighter(self.ide_panel.document())
        
        search.filterChanged.connect(self.highlighter.setSearchString)

        self.parseDefines(config_file_text)
        self.ide_panel.document().setPlainText(config_file_text)

        cursor = self.ide_panel.textCursor()
        cursor.setPosition(0)
        self.ide_panel.setTextCursor(cursor)
        self.ide_panel.setFocus()


        self.setLayout(layout)



    def getName(self):
        return "Configuration"


    def save(self):
        backup_path = "%s.backup" % self.config_file_path
        shutil.copyfile(self.config_file_path, backup_path)

        with open(self.config_file_path, "w") as f:
            f.write(self.ide_panel.getText())

        message = "To make your changes current, a reload of the configuration file is required. Would you like to reload now?"
        result = QMessageBox.information(self, "Reload required!", message, QMessageBox.Yes | QMessageBox.No)

        if result == QMessageBox.Yes:
            self.reload(self.config_file_path)


    def saveAs(self):
        config_file = QFileDialog.getSaveFileName(self, "Save Configuration File As")

        config_file = str(config_file)

        if len(config_file) > 0:
            with open(config_file, "w") as f:
                f.write(self.ide_panel.getText())

            message = "The current configuration file has been saved to a new file. Do you want to restart Ert using the new configuration file?"
            result = QMessageBox.information(self, "Restart Ert?", message, QMessageBox.Yes | QMessageBox.No)

            if result == QMessageBox.Yes:
                self.reload(config_file)


    def reload(self, path):
        self.reloadApplication.emit(path)

    def start(self):
        print("Start!")

    def parseDefines(self, text):
        pattern = re.compile("[ \t]*DEFINE[ \t]*(\S+)[ \t]*(\S+)")

        match = re.findall(pattern, text)

        for m in match:
            PathArgument.addDefine(m[0], m[1])

import shutil
from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget, QVBoxLayout, QToolBar, QStyle, QMessageBox, QSizePolicy
from ert_gui.ide.highlighter import KeywordHighlighter
from ert_gui.tools.ide import IdePanel
from ert_gui.widgets.search_box import SearchBox


class ConfigurationPanel(QWidget):

    reloadApplication = pyqtSignal()

    def __init__(self, config_file_path):
        QWidget.__init__(self)

        layout = QVBoxLayout()

        toolbar = QToolBar("toolbar")

        # start_icon = toolbar.style().standardIcon(QStyle.SP_MediaPlay)
        # start_action = toolbar.addAction(start_icon, "Run simulations")
        # start_action.triggered.connect(self.start)
        #
        # toolbar.addSeparator()

        save_icon = toolbar.style().standardIcon(QStyle.SP_DialogSaveButton)
        save_action = toolbar.addAction(save_icon, "Save")
        save_action.triggered.connect(self.save)

        # reload_icon = toolbar.style().standardIcon(QStyle.SP_BrowserReload)
        # reload_action = toolbar.addAction(reload_icon, "Reload")
        # reload_action.triggered.connect(self.reload)

        toolbar.addSeparator()

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

        self.ide_panel.document().setPlainText(config_file_text)

        cursor = self.ide_panel.textCursor()
        cursor.setPosition(0)
        self.ide_panel.setTextCursor(cursor)
        self.ide_panel.setFocus()


        self.setLayout(layout)

        # self.addLabeledSeparator("Case initialization")
        # case_combo = ComboChoice(CaseSelectorModel(), "Current case", "init/current_case_selection")
        # case_configurator = CaseInitializationConfigurationPanel()
        # self.addRow(case_combo, case_configurator)
        #
        # self.addLabeledSeparator("Queue System")
        #
        # queue_system_selector = QueueSystemSelector()
        # queue_system_combo = ComboChoice(queue_system_selector, "Queue system", "config/queue_system/queue_system")
        # queue_system_configurator = QueueSystemConfigurationPanel()
        # self.addRow(queue_system_combo, queue_system_configurator)


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
            self.reload()


    def reload(self):
        self.reloadApplication.emit()

    def start(self):
        print("Start!")

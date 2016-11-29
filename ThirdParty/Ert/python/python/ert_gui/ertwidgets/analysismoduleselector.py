from PyQt4.QtCore import QMargins, Qt
from PyQt4.QtGui import QWidget, QHBoxLayout, QComboBox, QToolButton

from ert_gui.ertwidgets import addHelpToWidget, ClosableDialog, resourceIcon
from ert_gui.ertwidgets.models.ertmodel import getCurrentAnalysisModuleName, getAnalysisModuleNames
from ert_gui.ertwidgets.analysismodulevariablespanel import AnalysisModuleVariablesPanel


class AnalysisModuleSelector(QWidget):
    def __init__(self, iterable=False, help_link=""):
        QWidget.__init__(self)
        self._iterable = iterable

        addHelpToWidget(self, help_link)

        layout = QHBoxLayout()

        analysis_module_combo = QComboBox()

        module_names = getAnalysisModuleNames(self._iterable)
        for module_name in module_names:
            analysis_module_combo.addItem(module_name)

        self._current_module_name = self._getCurrentAnalysisModuleName()
        if self._current_module_name is not None:
            analysis_module_combo.setCurrentIndex(module_names.index(self._current_module_name))

        analysis_module_combo.currentIndexChanged[int].connect(self.analysisModuleChanged)

        variables_popup_button = QToolButton()
        variables_popup_button.setIcon(resourceIcon("ide/small/cog_edit.png"))
        variables_popup_button.clicked.connect(self.showVariablesPopup)
        variables_popup_button.setMaximumSize(20, 20)

        layout.addWidget(analysis_module_combo, 0, Qt.AlignLeft)
        layout.addWidget(variables_popup_button, 0, Qt.AlignLeft)
        layout.setContentsMargins(QMargins(0, 0, 0, 0))
        layout.addStretch()

        self.setLayout(layout)

    def analysisModuleChanged(self, index):
        modules = getAnalysisModuleNames(self._iterable)
        self._current_module_name = modules[index]

    def _getCurrentAnalysisModuleName(self):
        active_name = getCurrentAnalysisModuleName()
        modules = getAnalysisModuleNames(self._iterable)

        if active_name in modules:
            return active_name
        elif "STD_ENKF" in modules and not self._iterable:
            return "STD_ENKF"
        elif "RML_ENKF" in modules and self._iterable:
            return "RML_ENKF"
        elif len(modules) > 0:
            return modules[0]

        return None

    def getSelectedAnalysisModuleName(self):
        return self._current_module_name

    def showVariablesPopup(self):
        if self.getSelectedAnalysisModuleName() is not None:
            variable_dialog = AnalysisModuleVariablesPanel(self.getSelectedAnalysisModuleName())
            dialog = ClosableDialog("Edit variables", variable_dialog, self.parent())

            dialog.exec_()

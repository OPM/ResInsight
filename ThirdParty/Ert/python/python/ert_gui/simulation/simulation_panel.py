from PyQt4.QtCore import Qt, QSize
from PyQt4.QtGui import QWidget, QVBoxLayout, QHBoxLayout, QLabel, QStackedWidget, QFrame, QToolButton, QMessageBox, QComboBox

from ert_gui import ERT
from ert_gui.ertwidgets import addHelpToWidget, resourceIcon
from ert_gui.ertwidgets.models.ertmodel import getCurrentCaseName
from ert_gui.simulation import EnsembleExperimentPanel, EnsembleSmootherPanel
from ert_gui.simulation import IteratedEnsembleSmootherPanel, MultipleDataAssimilationPanel, SimulationConfigPanel
from ert_gui.simulation import RunDialog
from collections import OrderedDict

class SimulationPanel(QWidget):

    def __init__(self):
        QWidget.__init__(self)

        layout = QVBoxLayout()

        self._simulation_mode_combo = QComboBox()
        addHelpToWidget(self._simulation_mode_combo, "run/simulation_mode")

        self._simulation_mode_combo.currentIndexChanged.connect(self.toggleSimulationMode)

        simulation_mode_layout = QHBoxLayout()
        simulation_mode_layout.addSpacing(10)
        simulation_mode_layout.addWidget(QLabel("Simulation mode:"), 0, Qt.AlignVCenter)
        simulation_mode_layout.addWidget(self._simulation_mode_combo, 0, Qt.AlignVCenter)

        simulation_mode_layout.addSpacing(20)

        self.run_button = QToolButton()
        self.run_button.setIconSize(QSize(32, 32))
        self.run_button.setText("Start Simulation")
        self.run_button.setIcon(resourceIcon("ide/gear_in_play"))
        self.run_button.clicked.connect(self.runSimulation)
        self.run_button.setToolButtonStyle(Qt.ToolButtonTextBesideIcon)
        addHelpToWidget(self.run_button, "run/start_simulation")

        simulation_mode_layout.addWidget(self.run_button)
        simulation_mode_layout.addStretch(1)

        layout.addSpacing(5)
        layout.addLayout(simulation_mode_layout)
        layout.addSpacing(10)

        self._simulation_stack = QStackedWidget()
        self._simulation_stack.setLineWidth(1)
        self._simulation_stack.setFrameStyle(QFrame.StyledPanel)

        layout.addWidget(self._simulation_stack)

        self._simulation_widgets = OrderedDict()
        """ :type: OrderedDict[BaseRunModel,SimulationConfigPanel]"""

        self.addSimulationConfigPanel(EnsembleExperimentPanel())
        self.addSimulationConfigPanel(EnsembleSmootherPanel())
        self.addSimulationConfigPanel(IteratedEnsembleSmootherPanel(advanced_option=True))
        self.addSimulationConfigPanel(MultipleDataAssimilationPanel())

        self.setLayout(layout)


    def addSimulationConfigPanel(self, panel):
        assert isinstance(panel, SimulationConfigPanel)

        panel.toggleAdvancedOptions(False)
        self._simulation_stack.addWidget(panel)

        simulation_model = panel.getSimulationModel()

        self._simulation_widgets[simulation_model] = panel

        if not panel.is_advanced_option:
            self._simulation_mode_combo.addItem(str(simulation_model), simulation_model)

        panel.simulationConfigurationChanged.connect(self.validationStatusChanged)


    def getActions(self):
        return []

    def toggleAdvancedOptions(self, show_advanced):
        current_model = self.getCurrentSimulationModel()

        self._simulation_mode_combo.clear()

        for model, panel in self._simulation_widgets.iteritems():
            if show_advanced or not panel.is_advanced_option:
                self._simulation_mode_combo.addItem(str(model), model)

        old_index = self._simulation_mode_combo.findText(str(current_model))
        self._simulation_mode_combo.setCurrentIndex(old_index if old_index > -1 else 0)

    def toggleAdvancedMode(self, show_advanced):
        for panel in self._simulation_widgets.values():
            panel.toggleAdvancedOptions(show_advanced)

        self.toggleAdvancedOptions(show_advanced)

    def getCurrentSimulationModel(self):
        data = self._simulation_mode_combo.itemData(self._simulation_mode_combo.currentIndex(), Qt.UserRole)
        return data.toPyObject()

    def getSimulationArguments(self):
        """ @rtype: dict[str,object]"""
        simulation_widget = self._simulation_widgets[self.getCurrentSimulationModel()]
        return simulation_widget.getSimulationArguments()


    def runSimulation(self):
        case_name = getCurrentCaseName()
        message = "Are you sure you want to use case '%s' for initialization of the initial ensemble when running the simulations?" % case_name
        start_simulations = QMessageBox.question(self, "Start simulations?", message, QMessageBox.Yes | QMessageBox.No )

        if start_simulations == QMessageBox.Yes:
            run_model = self.getCurrentSimulationModel()
            arguments = self.getSimulationArguments()
            dialog = RunDialog(run_model, arguments, self)
            dialog.startSimulation()
            dialog.exec_()

            ERT.emitErtChange() # simulations may have added new cases.


    def toggleSimulationMode(self):
        current_model = self.getCurrentSimulationModel()
        if current_model is not None:
            widget = self._simulation_widgets[self.getCurrentSimulationModel()]
            self._simulation_stack.setCurrentWidget(widget)
            self.validationStatusChanged()


    def validationStatusChanged(self):
        widget = self._simulation_widgets[self.getCurrentSimulationModel()]
        self.run_button.setEnabled(widget.isConfigurationValid())

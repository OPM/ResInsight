from ert_gui.models.connectors import RunPathModel, EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel, IsCaseInitializedModel
from ert_gui.models.connectors.run import SimulationModeModel, OneMoreIteration
from ert_gui.models.mixins.connectorless import FunctionButtonModel
from ert_gui.pages.message_center import MessageCenter
from ert_gui.models.connectors.run.simulation_runner import SimulationRunner
from ert_gui.pages.run_dialog import RunDialog
from ert_gui.widgets.button import Button
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.integer_spinner import IntegerSpinner
from ert_gui.widgets.path_format import PathFormatChooser
from ert_gui.widgets.row_panel import RowPanel
from ert_gui.widgets.warning_panel import WarningPanel




class SimulationPanel(RowPanel):

    def __init__(self):
        RowPanel.__init__(self, "Simulation")

        self.addLabeledSeparator("Simulation settings")

        case_model = CaseSelectorModel()
        case_selector = ComboChoice(case_model, "Current case", "init/current_case_selection")
        self.addRow(case_selector)

        # Give a warning if the case is not initialized!
        IsCaseInitializedModel().observable().attach(IsCaseInitializedModel.TEXT_VALUE_CHANGED_EVENT, self.updateSimulationStatus)

        runpath_model = RunPathModel()
        self.addRow(PathFormatChooser(runpath_model, "Runpath", "config/simulation/runpath"))

        ensemble_size_model = EnsembleSizeModel()
        self.addRow(IntegerSpinner(ensemble_size_model, "Number of realizations", "config/ensemble/num_realizations"))

        simulation_mode_model = SimulationModeModel()
        self.addRow(ComboChoice(simulation_mode_model, "Simulation mode", "run/simulation_mode"))

        self.addSpace(10)
        self.run = FunctionButtonModel("Run", self.runSimulation)
        self.run.setEnabled(SimulationModeModel().getCurrentChoice().buttonIsEnabled())

        self.run_button = Button(self.run, label="Start simulation", help_link="run/run")
        self.run_button.addStretch()

        self.config_and_run = FunctionButtonModel("Configure and Run", self.configureAndRunSimulation)
        self.config_and_run.setEnabled(SimulationModeModel().getCurrentChoice().buttonIsEnabled())

        self.run_button.addOption(self.config_and_run)
        self.run_button.addOption(OneMoreIteration())
        self.addRow(self.run_button)
        self.addSpace(10)

        self.warning_panel = WarningPanel()
        self.addRow(self.warning_panel)

        simulation_mode_model.observable().attach(SimulationModeModel.CURRENT_CHOICE_CHANGED_EVENT, self.toggleSimulationMode)

        self.updateSimulationStatus()


    def updateSimulationStatus(self):
        MessageCenter().setWarning(IsCaseInitializedModel.__name__, IsCaseInitializedModel().getText())
        #todo move this binding to the model?

    def runSimulation(self):
        simulation_model = SimulationModeModel().getCurrentChoice()

        simulation_runner = SimulationRunner(simulation_model)


        dialog = RunDialog(simulation_runner)
        simulation_runner.start()
        dialog.exec_()

        self.updateSimulationStatus()


    def configureAndRunSimulation(self):
        simulation_model = SimulationModeModel().getCurrentChoice()
        print("Configure!!!")
        simulation_model.buttonTriggered()


    def toggleSimulationMode(self):
        model = SimulationModeModel().getCurrentChoice()
        self.run.setEnabled(model.buttonIsEnabled())
        self.config_and_run.setEnabled(model.buttonIsEnabled())





from PyQt4.QtGui import QFormLayout
from ert_gui.ide.keywords.definitions import RangeStringArgument
from ert_gui.models.connectors import EnsembleSizeModel
from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.run import EnsembleExperiment, ActiveRealizationsModel, RunPathModel
from ert_gui.simulation.simulation_config_panel import SimulationConfigPanel
from ert_gui.widgets.active_label import ActiveLabel
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.string_box import StringBox


class EnsembleExperimentPanel(SimulationConfigPanel):

    def __init__(self):
        SimulationConfigPanel.__init__(self, EnsembleExperiment())

        layout = QFormLayout()

        case_model = CaseSelectorModel()
        case_selector = ComboChoice(case_model, "Current case", "init/current_case_selection")
        layout.addRow(case_selector.getLabel(), case_selector)

        runpath_model = RunPathModel()
        runpath_label = ActiveLabel(runpath_model, "Runpath", "config/simulation/runpath")
        layout.addRow(runpath_label.getLabel(), runpath_label)


        number_of_realizations_model = EnsembleSizeModel()
        number_of_realizations_label = ActiveLabel(number_of_realizations_model, "Number of realizations", "config/ensemble/num_realizations")
        layout.addRow(number_of_realizations_label.getLabel(), number_of_realizations_label)

        active_realizations_model = ActiveRealizationsModel()
        self.active_realizations_field = StringBox(active_realizations_model, "Active realizations", "config/simulation/active_realizations")
        self.active_realizations_field.setValidator(RangeStringArgument(number_of_realizations_model.getValue()))
        layout.addRow(self.active_realizations_field.getLabel(), self.active_realizations_field)

        self.active_realizations_field.validationChanged.connect(self.simulationConfigurationChanged)


        self.setLayout(layout)

    def isConfigurationValid(self):
        return self.active_realizations_field.isValid()







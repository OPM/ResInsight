from PyQt4.QtGui import QFormLayout, QLabel

from ert_gui.ertwidgets import addHelpToWidget
from ert_gui.ertwidgets.caseselector import CaseSelector
from ert_gui.ertwidgets.models.activerealizationsmodel import ActiveRealizationsModel
from ert_gui.ertwidgets.models.ertmodel import getRealizationCount, getRunPath
from ert_gui.ertwidgets.stringbox import StringBox
from ert_gui.ide.keywords.definitions import RangeStringArgument
from ert_gui.simulation.models import EnsembleExperiment
from ert_gui.simulation.simulation_config_panel import SimulationConfigPanel


class EnsembleExperimentPanel(SimulationConfigPanel):

    def __init__(self):
        SimulationConfigPanel.__init__(self, EnsembleExperiment())

        layout = QFormLayout()

        case_selector = CaseSelector()
        layout.addRow("Current case:", case_selector)

        run_path_label = QLabel("<b>%s</b>" % getRunPath())
        addHelpToWidget(run_path_label, "config/simulation/runpath")
        layout.addRow("Runpath:", run_path_label)

        number_of_realizations_label = QLabel("<b>%d</b>" % getRealizationCount())
        addHelpToWidget(number_of_realizations_label, "config/ensemble/num_realizations")
        layout.addRow(QLabel("Number of realizations:"), number_of_realizations_label)

        self._active_realizations_model = ActiveRealizationsModel()
        self._active_realizations_field = StringBox(self._active_realizations_model, "config/simulation/active_realizations")
        self._active_realizations_field.setValidator(RangeStringArgument(getRealizationCount()))
        layout.addRow("Active realizations", self._active_realizations_field)

        self._active_realizations_field.getValidationSupport().validationChanged.connect(self.simulationConfigurationChanged)

        self.setLayout(layout)


    def isConfigurationValid(self):
        return self._active_realizations_field.isValid()

    def toggleAdvancedOptions(self, show_advanced):
        self._active_realizations_field.setVisible(show_advanced)
        self.layout().labelForField(self._active_realizations_field).setVisible(show_advanced)

    def getSimulationArguments(self):
        active_realizations_mask = self._active_realizations_model.getActiveRealizationsMask()
        return {"active_realizations": active_realizations_mask}



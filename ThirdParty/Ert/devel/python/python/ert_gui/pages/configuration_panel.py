from ert_gui.models.connectors.init import CaseSelectorModel
from ert_gui.models.connectors.queue_system.queue_system_selector import QueueSystemSelector
from ert_gui.pages.case_init_configuration import CaseInitializationConfigurationPanel
from ert_gui.pages.queue_system_configuration import QueueSystemConfigurationPanel
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.row_panel import RowPanel



class ConfigurationPanel(RowPanel):

    def __init__(self):
        RowPanel.__init__(self, "Configuration")

        self.addLabeledSeparator("Case initialization")
        case_combo = ComboChoice(CaseSelectorModel(), "Current case", "init/current_case_selection")
        case_configurator = CaseInitializationConfigurationPanel()
        self.addRow(case_combo, case_configurator)

        self.addLabeledSeparator("Queue System")

        queue_system_selector = QueueSystemSelector()
        queue_system_combo = ComboChoice(queue_system_selector, "Queue system", "config/queue_system/queue_system")
        queue_system_configurator = QueueSystemConfigurationPanel()
        self.addRow(queue_system_combo, queue_system_configurator)





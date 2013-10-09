from ert_gui.models.connectors.queue_system import LsfMaxRunning, LsfQueue, LsfRequest, LocalMaxRunning, RshMaxRunning, RshCommand, RshHostListModel
from ert_gui.widgets.integer_spinner import IntegerSpinner
from ert_gui.widgets.keyword_table import KeywordTable
from ert_gui.widgets.path_chooser import PathChooser
from ert_gui.widgets.row_panel import RowPanel
from ert_gui.widgets.string_box import StringBox


class QueueSystemConfigurationPanel(RowPanel):

    def __init__(self):
        RowPanel.__init__(self, "Queue System")

        # self.startTabs("LSF")
        self.addLabeledSeparator("LSF")
        self.addRow(StringBox(LsfQueue(), "LSF Queue", "config/queue_system/lsf_queue"))
        self.addRow(IntegerSpinner(LsfMaxRunning(), "Max running", "config/queue_system/max_running_lsf"))
        self.addRow(StringBox(LsfRequest(), "Resources", "config/queue_system/lsf_resources"))
        self.addSpace(10)

        # self.addTab("RSH")
        self.addLabeledSeparator("RSH")
        self.addRow(PathChooser(RshCommand(), "Command", "config/queue_system/rsh_command"))
        self.addRow(IntegerSpinner(RshMaxRunning(), "Max running", "config/queue_system/max_running_rsh"))

        keyword_table = KeywordTable(RshHostListModel(), "Host List", "config/queue_system/rsh_host_list")
        keyword_table.setColumnHeaders(keyword_name="Host", value_name="Number of Jobs")
        self.addRow(keyword_table)
        self.addSpace(10)

        # self.addTab("LOCAL")
        self.addLabeledSeparator("Local")
        self.addRow(IntegerSpinner(LocalMaxRunning(), "Max running", "config/queue_system/max_running_local"))
        self.addSpace(20)

        # self.endTabs()


from PyQt4.QtGui import QToolButton
from ert_gui.models.connectors.init import InitializeFromScratchModel
from ert_gui.models.connectors.init.case_list import CaseList
from ert_gui.models.connectors.init.init_from_existing import InitializeFromExistingCaseModel
from ert_gui.models.connectors.init.init_history_length import HistoryLengthModel
from ert_gui.models.connectors.init.init_members import InitializationMembersModel
from ert_gui.models.connectors.init.init_parameters import InitializationParametersModel
from ert_gui.models.connectors.init.initialized_case_selector import InitializedCaseSelectorModel
from ert_gui.widgets.button import Button
from ert_gui.widgets.check_list import CheckList
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.integer_spinner import IntegerSpinner
from ert_gui.widgets.keyword_list import KeywordList
from ert_gui.widgets.row_group import RowGroup
from ert_gui.widgets.row_panel import RowPanel
from ert_gui.widgets.validated_dialog import ValidatedDialog


class CaseInitializationConfigurationPanel(RowPanel):

    def __init__(self):
        RowPanel.__init__(self, "Case Management")

        self.addCreateNewCaseTab()
        self.addInitializeFromScratchTab()
        self.addInitializeFromExistingTab()

        self.endTabs()

    def newValidatedKeywordPopup(self, existing_keywords):
        dialog = ValidatedDialog("New case", "Enter name of new case:", existing_keywords)
        return dialog.showAndTell()

    def addCreateNewCaseTab(self):
        self.startTabs("Create new case")

        case_list = KeywordList(CaseList(), "Available cases", "init/case_list")
        case_list.setMaximumWidth(250)
        case_list.newKeywordPopup = self.newValidatedKeywordPopup
        case_list.setSelectable(False)

        self.addRow(case_list)


    def addInitializeFromScratchTab(self):
        self.addTab("Initialize from scratch")

        row_group = RowGroup()
        parameter_model = InitializationParametersModel()
        parameter_check_list = CheckList(parameter_model, "Parameters", "init/select_parameters")
        parameter_check_list.setMaximumWidth(300)
        row_group.addWidget(parameter_check_list)

        member_model = InitializationMembersModel()
        member_check_list = CheckList(member_model, "Members", "init/select_members")
        member_check_list.setMaximumWidth(150)
        row_group.addWidget(member_check_list)

        self.addRow(row_group)

        self.addSpace(10)

        initialize_from_scratch = InitializeFromScratchModel()
        self.addRow(Button(initialize_from_scratch, help_link="init/initialize_from_scratch"))

        self.addSpace(10)


    def addInitializeFromExistingTab(self):
        self.addTab("Initialize from existing")


        initialized_cases = ComboChoice(InitializedCaseSelectorModel(), "Source case", "init/source_case")
        self.addRow(initialized_cases)

        #self.addRow("State", "Analyzed/Forecast")

        timestep_group = RowGroup("Timestep")
        history_length = HistoryLengthModel()
        history_length_spinner = IntegerSpinner(history_length, "Timestep", "config/init/history_length")
        timestep_group.addWidget(history_length_spinner)

        initial = QToolButton()
        initial.setText("Initial")
        initial.clicked.connect(history_length.setToMin)
        timestep_group.addWidget(initial)

        end_of_time = QToolButton()
        end_of_time.setText("End of time")
        end_of_time.clicked.connect(history_length.setToMax)
        timestep_group.addWidget(end_of_time)

        self.addRow(timestep_group)

        self.addSpace(10)

        row_group = RowGroup()
        parameter_model = InitializationParametersModel()
        parameter_check_list = CheckList(parameter_model, "Parameters", "init/select_parameters")
        parameter_check_list.setMaximumWidth(300)
        row_group.addWidget(parameter_check_list)

        member_model = InitializationMembersModel()
        member_check_list = CheckList(member_model, "Members", "init/select_members")
        member_check_list.setMaximumWidth(150)
        row_group.addWidget(member_check_list)

        self.addRow(row_group)

        self.addSpace(10)

        initialize_from_existing = InitializeFromExistingCaseModel()
        self.addRow(Button(initialize_from_existing, help_link="init/initialize_from_existing"))

        self.addSpace(10)

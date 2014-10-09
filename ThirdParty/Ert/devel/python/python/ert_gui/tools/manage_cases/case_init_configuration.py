from PyQt4.QtGui import QToolButton, QComboBox, QTextEdit
from ert_gui.models.connectors.init import InitializeFromScratchModel, CaseSelectorModel
from ert_gui.models.connectors.init.case_list import CaseList
from ert_gui.models.connectors.init.init_from_existing import InitializeFromExistingCaseModel
from ert_gui.models.connectors.init.init_history_length import HistoryLengthModel
from ert_gui.models.connectors.init.init_members import InitializationMembersModel
from ert_gui.models.connectors.init.init_parameters import InitializationParametersModel
from ert_gui.models.connectors.init.initialized_case_selector import InitializedCaseSelectorModel
from ert_gui.models.qt.all_cases_model import AllCasesModel
from ert_gui.widgets.button import Button
from ert_gui.widgets.check_list import CheckList
from ert_gui.widgets.combo_choice import ComboChoice
from ert_gui.widgets.helped_widget import HelpedWidget
from ert_gui.widgets.integer_spinner import IntegerSpinner
from ert_gui.widgets.keyword_list import KeywordList
from ert_gui.widgets.row_group import RowGroup
from ert_gui.widgets.row_panel import RowPanel
from ert_gui.widgets.util import may_take_a_long_time
from ert_gui.widgets.validated_dialog import ValidatedDialog


class CaseInitializationConfigurationPanel(RowPanel):

    @may_take_a_long_time
    def __init__(self):
        RowPanel.__init__(self, "Case Management")
        self.setMinimumWidth(600)

        self.addCreateNewCaseTab()
        self.addInitializeFromScratchTab()
        self.addInitializeFromExistingTab()
        self.addShowCaseInfo()

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

        case_model = CaseSelectorModel()
        case_selector = ComboChoice(case_model, "Target case", "init/current_case_selection")
        self.addRow(case_selector)

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

        case_model = CaseSelectorModel()
        target_case_selector = ComboChoice(case_model, "Target case", "init/current_case_selection")
        self.addRow(target_case_selector)

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

        timestep_group.addGroupStretch()

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

    def addShowCaseInfo(self):
        self.addTab("Case Info")

        case_widget = HelpedWidget("Select case", "init/select_case_for_info")

        model = AllCasesModel()
        self.combo = QComboBox()
        self.combo.setSizeAdjustPolicy(QComboBox.AdjustToMinimumContentsLength)
        self.combo.setMinimumContentsLength(20)
        self.combo.setModel(model)
        self.combo.currentIndexChanged.connect(self.showInfoForCase)

        case_widget.addWidget(self.combo)
        case_widget.addStretch()
        self.addRow(case_widget)


        area_widget = HelpedWidget("Case info", "init/selected_case_info")

        self.text_area = QTextEdit()
        self.text_area.setReadOnly(True)
        self.text_area.setMinimumHeight(300)

        area_widget.addWidget(self.text_area)
        area_widget.addStretch()
        self.addRow(area_widget)

        choice = CaseSelectorModel().getCurrentChoice()
        self.combo.setCurrentIndex(model.indexOf(choice))


    def showInfoForCase(self):
        case = self.combo.currentText()

        states = CaseList().getCaseRealizationStates(str(case))

        html = "<table>"
        for index in range(len(states)):
            html += "<tr><td width=30>%d.</td><td>%s</td></tr>" % (index, str(states[index]))

        html += "</table>"


        self.text_area.setHtml(html)


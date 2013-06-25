#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'initandcopy.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 


from ert_gui.widgets.helpedwidget import HelpedWidget
from PyQt4 import QtGui, QtCore
from ert_gui.widgets.util import resourceIcon, ListCheckPanel, ValidatedTimestepCombo, getItemsFromList
from ert.ert.enums import ert_state_enum

class ParametersAndMembers(HelpedWidget):

    listOfParameters = []
    listOfDynamicParameters = []
    maxTimeStep = 11


    def __init__(self, parent = None):
        HelpedWidget.__init__(self, parent)

        radioLayout = self.createRadioButtons()
        listLayout = self.createParameterMemberPanel()
        stLayout = self.createSourceTargetLayout()
        actionLayout = self.createActionButton()

        layout = QtGui.QVBoxLayout()
        layout.addLayout(radioLayout)
        layout.addSpacing(5)
        layout.addLayout(listLayout)
        layout.addSpacing(5)
        layout.addLayout(stLayout)
        layout.addSpacing(5)
        layout.addLayout(actionLayout)

        self.addLayout(layout)

        self.initialized = False     
        self.modelConnect("casesUpdated()", self.fetchContent)
        self.modelConnect("ensembleResized()", self.fetchContent)
        self.modelConnect("ensembleUpdated()", self.fetchContent)

        self.toggleScratch.toggle()


    def toggleCompleteEnsembleState(self, checkState):
        self.parametersList.setSelectionEnabled(not checkState)

        if checkState:
            self.parametersList.selectAll()


    def toggleActionState(self, action="Initialize", showCopyParameters = False, selectSource = False, selectTarget = False):
        self.sourceLabel.setEnabled(selectSource)
        self.sourceCase.setEnabled(selectSource)
        self.sourceType.setEnabled(selectSource)
        self.sourceReportStep.setEnabled(selectSource)
        self.sourceCompleteEnsembleCheck.setEnabled(showCopyParameters)

        if not selectSource:
            self.sourceReportStep.setCurrentIndex(0)

        self.targetLabel.setEnabled(selectTarget)
        self.targetCaseLabel.setEnabled(selectTarget)
        self.targetType.setEnabled(selectTarget)
        self.targetReportStep.setEnabled(selectTarget)


        if not selectTarget:
            self.targetReportStep.setCurrentIndex(0)

        self.actionButton.setText(action)


        self.parametersList.clear()
        self.parametersList.addItems(self.listOfParameters)

        self.parametersList.setEnabled(True)
        self.parametersList.checkAll.setEnabled(True)
        self.parametersList.uncheckAll.setEnabled(True)


        if showCopyParameters:
            self.parametersList.addItems(self.listOfDynamicParameters)
            self.toggleCompleteEnsembleState(self.sourceCompleteEnsembleCheck.isChecked())

        self.parametersList.selectAll()
        self.membersList.selectAll()


    def initializeCaseFromScratch(self, parameters, members):
        ert = self.getModel()

        stringlist = ert.createStringList(parameters)

        for member in members:
            m = int(member.strip())
            ert.enkf.enkf_main_initialize_from_scratch(ert.main, stringlist, m , m)

        ert.freeStringList(stringlist)

    def initializeCaseFromCase(self, selected_parameters, selected_members):
        ert = self.getModel()

        selected_parameters = [str(parameter) for parameter in selected_parameters]
        selected_members = [int(member.strip()) for member in selected_members]

        source_case = str(self.sourceCase.currentText())
        source_report_step = self.sourceReportStep.getSelectedValue()
        source_state = ert_state_enum.resolveName(str(self.sourceType.currentText())).value()
        member_mask = ert.createBoolVector(self.membersList.count(), selected_members)
        ranking_key = None
        node_list = ert.createStringList(selected_parameters)

        ert.enkf.enkf_main_initialize_from_existing__(ert.main,
                                                      source_case,
                                                      source_report_step,
                                                      source_state,
                                                      member_mask,
                                                      ranking_key,
                                                      node_list)

        ert.freeStringList(node_list)
        ert.freeBoolVector(member_mask)

    def copyEnsemble(self, selected_parameters, selected_members):
        ert = self.getModel()

        selected_parameters = [str(parameter) for parameter in selected_parameters]
        selected_members = [int(member.strip()) for member in selected_members]

        source_case = str(self.sourceCase.currentText())
        source_report_step = self.sourceReportStep.getSelectedValue()
        source_state = ert_state_enum.resolveName(str(self.sourceType.currentText())).value()

        target_case = str(self.targetCaseLabel.text())
        target_report_step = self.targetReportStep.getSelectedValue()
        target_state = ert_state_enum.resolveName(str(self.targetType.currentText())).value()

        member_mask = ert.createBoolVector(self.membersList.count(), selected_members)
        ranking_key = None
        node_list = ert.createStringList(selected_parameters)

        ert.enkf.enkf_main_copy_ensemble(ert.main,
                                         source_case,
                                         source_report_step,
                                         source_state,
                                         target_case,
                                         target_report_step,
                                         target_state,
                                         member_mask,
                                         ranking_key,
                                         node_list)

        ert.freeStringList(node_list)
        ert.freeBoolVector(member_mask)

    def initializeOrCopy(self):
        selected_parameters = getItemsFromList(self.parametersList)
        selected_members = getItemsFromList(self.membersList)

        if len(selected_parameters) == 0 or len(selected_members) == 0:
            QtGui.QMessageBox.warning(self, "Missing data", "At least one parameter and one member must be selected!")
            return

        if self.toggleScratch.isChecked():
            self.initializeCaseFromScratch(selected_parameters, selected_members)
        elif self.toggleInitCopy.isChecked():
            self.initializeCaseFromCase(selected_parameters, selected_members)
        else:
            self.copyEnsemble(selected_parameters, selected_members)


    def fetchContent(self):
        data = self.getFromModel()

        self.parametersList.clear()
        self.membersList.clear()
        self.sourceCase.clear()

        self.listOfParameters = data["parameters"]
        self.listOfDynamicParameters = data["dynamic_parameters"]


        for member in data["members"]:
            self.membersList.addItem("%03d" % (member))
            #self.membersList.addItem(str(member))

        for case in data["cases"]:
            if not case == data["current_case"]:
                self.sourceCase.addItem(case)

        self.maxTimeStep = data["history_length"]

        self.sourceReportStep.setHistoryLength(self.maxTimeStep)
        self.targetReportStep.setHistoryLength(self.maxTimeStep)

        self.targetCaseLabel.setText(data["current_case"])

        if self.toggleScratch.isChecked():
            self.toggleScratch.emit(QtCore.SIGNAL("toggled(bool)"), True)
        elif self.toggleInitCopy.isChecked():
            self.toggleInitCopy.emit(QtCore.SIGNAL("toggled(bool)"), True)
        else:
            self.toggleCopy.emit(QtCore.SIGNAL("toggled(bool)"), True)


    def initialize(self, ert):
        self.initialized = True


    def getter(self, ert):
        if not self.initialized:
            self.initialize(ert)
            
        #enums from enkf_types.h
        PARAMETER = 1
        DYNAMIC_STATE = 2
        keylist = ert.enkf.ensemble_config_alloc_keylist_from_var_type(ert.ensemble_config, PARAMETER )
        
        parameters = ert.getStringList(keylist)
        ert.freeStringList(keylist)

        keylist = ert.enkf.ensemble_config_alloc_keylist_from_var_type(ert.ensemble_config,  DYNAMIC_STATE )
        dynamicParameters = ert.getStringList(keylist)
        ert.freeStringList(keylist)

        members = ert.enkf.enkf_main_get_ensemble_size(ert.main)

        fs = ert.enkf.enkf_main_get_fs(ert.main)
        currentCase = "default" #ert.enkf.enkf_fs_get_read_dir(fs)

        #caseList = ert.enkf.enkf_fs_alloc_dirlist(fs)
        #list = ert.getStringList(caseList)
        #ert.freeStringList(caseList)
        list = ["default"]
        historyLength = ert.enkf.enkf_main_get_history_length(ert.main)

        return {"parameters" : parameters,
                "dynamic_parameters" : dynamicParameters,
                "members" : range(members),
                "current_case" : currentCase,
                "cases" : list,
                "history_length" : historyLength}


    def setter(self, ert, value):
        """The setting of these values are activated by a separate button."""
        pass


    def createCheckPanel(self, list):
        return ListCheckPanel(list)


    def createRadioButtons(self):
        radioLayout = QtGui.QVBoxLayout()
        radioLayout.setSpacing(2)
        self.toggleScratch = QtGui.QRadioButton("Initialize from scratch")
        radioLayout.addWidget(self.toggleScratch)
        self.toggleInitCopy = QtGui.QRadioButton("Initialize from existing case")
        radioLayout.addWidget(self.toggleInitCopy)
        self.toggleCopy = QtGui.QRadioButton("Copy from existing case")
        radioLayout.addWidget(self.toggleCopy)

        self.connect(self.toggleScratch, QtCore.SIGNAL('toggled(bool)'), lambda : self.toggleActionState())
        self.connect(self.toggleInitCopy, QtCore.SIGNAL('toggled(bool)'), lambda : self.toggleActionState(selectSource = True))
        self.connect(self.toggleCopy, QtCore.SIGNAL('toggled(bool)'), lambda : self.toggleActionState(action = "Copy", selectSource=True, showCopyParameters=True, selectTarget=True))

        return radioLayout


    def createParameterMemberPanel(self):
        self.parametersList = QtGui.QListWidget(self)
        self.parametersList.setSelectionMode(QtGui.QAbstractItemView.MultiSelection)
        self.membersList = QtGui.QListWidget(self)
        self.membersList.setSelectionMode(QtGui.QAbstractItemView.MultiSelection)

        #--- members iconview code ---
        self.membersList.setViewMode(QtGui.QListView.IconMode)
        self.membersList.setMovement(QtGui.QListView.Static)
        self.membersList.setResizeMode(QtGui.QListView.Adjust)
        #self.membersList.setUniformItemSizes(True)
        self.membersList.setGridSize(QtCore.QSize(32, 16))
        self.membersList.setSelectionRectVisible(False)
        #-----------------------------

        parameterLayout = QtGui.QVBoxLayout()
        parametersCheckPanel = self.createCheckPanel(self.parametersList)
        parametersCheckPanel.insertWidget(0, QtGui.QLabel("Parameters"))
        parameterLayout.addLayout(parametersCheckPanel)
        parameterLayout.addWidget(self.parametersList)

        memberLayout = QtGui.QVBoxLayout()
        membersCheckPanel = self.createCheckPanel(self.membersList)
        membersCheckPanel.insertWidget(0, QtGui.QLabel("Members"))
        memberLayout.addLayout(membersCheckPanel)
        memberLayout.addWidget(self.membersList)

        listLayout = QtGui.QHBoxLayout()
        listLayout.addLayout(parameterLayout)
        listLayout.addLayout(memberLayout)

        return listLayout


    def createActionButton(self):
        self.actionButton = QtGui.QPushButton("Initialize")

        self.connect(self.actionButton, QtCore.SIGNAL('clicked()'), self.initializeOrCopy)

        actionLayout = QtGui.QHBoxLayout()
        actionLayout.addStretch(1)
        actionLayout.addWidget(self.actionButton)
        actionLayout.addStretch(1)

        return actionLayout


    def createSourceTargetLayout(self):
        self.createSourceTargetWidgets()

        stLayout = QtGui.QGridLayout()
        stLayout.setColumnStretch(8, 1)
        stLayout.addWidget(QtGui.QLabel("Case"), 0, 1)
        stLayout.addWidget(QtGui.QLabel("State"), 0, 3)
        stLayout.addWidget(QtGui.QLabel("Timestep"), 0, 5)
        self.sourceLabel = QtGui.QLabel("Source:")
        stLayout.addWidget(self.sourceLabel, 1, 0)
        stLayout.addWidget(self.sourceCase, 1, 1)
        stLayout.addWidget(self.sourceType, 1, 3)
        stLayout.addWidget(self.sourceReportStep, 1, 5)
        stLayout.addWidget(self.sourceCompleteEnsembleCheck, 1, 7)

        self.targetCaseLabel = QtGui.QLabel("none?")
        font = self.targetCaseLabel.font()
        font.setWeight(QtGui.QFont.Bold)
        self.targetCaseLabel.setFont(font)

        self.targetLabel = QtGui.QLabel("Target:")

        stLayout.addWidget(self.targetLabel, 2, 0)
        stLayout.addWidget(self.targetCaseLabel, 2, 1)
        stLayout.addWidget(self.targetType, 2, 3)
        stLayout.addWidget(self.targetReportStep, 2, 5)

        return stLayout


    def createSourceTargetWidgets(self):
        self.sourceCase = QtGui.QComboBox(self)
        self.sourceCase.setMaximumWidth(150)
        self.sourceCase.setMinimumWidth(150)
        self.sourceCase.setToolTip("Select source case")
        self.sourceType = QtGui.QComboBox(self)
        self.sourceType.setMaximumWidth(100)
        self.sourceType.setToolTip("Select source state")
        for state in ert_state_enum.INITIALIZATION_STATES:
            self.sourceType.addItem(str(state))

        self.sourceReportStep = ValidatedTimestepCombo(self)
        self.sourceCompleteEnsembleCheck = QtGui.QCheckBox("Complete Ensemble")
        self.sourceCompleteEnsembleCheck.setChecked(True)

        self.connect(self.sourceCompleteEnsembleCheck, QtCore.SIGNAL('stateChanged(int)'),
                    lambda state : self.toggleCompleteEnsembleState(state == QtCore.Qt.Checked))

        self.targetType = QtGui.QComboBox(self)
        self.targetType.setMaximumWidth(100)
        self.targetType.setToolTip("Select target state")
        for state in ert_state_enum.INITIALIZATION_STATES:
            self.targetType.addItem(str(state))

        self.targetReportStep = ValidatedTimestepCombo(self)

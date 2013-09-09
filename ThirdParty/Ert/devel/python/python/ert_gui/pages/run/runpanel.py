#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'runpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4 import QtGui, QtCore

from ert_gui.widgets.helpedwidget import HelpedWidget, ContentModel
from ert_gui.widgets.util import resourceIcon, ListCheckPanel, ValidatedTimestepCombo, createSpace, getItemsFromList, frange
import threading
import time
#import widgets
import math
from ert_gui.widgets.cogwheel import Cogwheel
from simulation import SimulationList, SimulationItemDelegate, SimulationItem, Simulation
from legend import Legend
from simulationsdialog import SimulationsDialog

class RunWidget(HelpedWidget):
    """A widget that shows simulation parameters and the possibility to start the simulation"""
    run_mode_type = {"ENKF_ASSIMILATION" : 1, "ENSEMBLE_EXPERIMENT" : 2, "ENSEMBLE_PREDICTION" : 3, "INIT_ONLY" : 4, "SMOOTHER" : 5}
    state_enum = {"UNDEFINED" : 0, "SERIALIZED" : 1, "FORECAST" : 2, "ANALYZED" : 4, "BOTH" : 6}

    def __init__(self, parent=None):
        HelpedWidget.__init__(self, parent, widgetLabel="", helpLabel="") #

        self.addLayout(self.createPanel(parent))

        self.modelConnect("ensembleResized()", self.fetchContent)
        self.modelConnect("runpathChanged()", self.fetchContent)
        self.rbAssimilation.toggle()
        #self.rbExperiment.toggle()    
        #self.rbSmoother.toggle()

    def createPanel(self, parent):
        """Creates the panel with the simulation parameters."""
        self.membersList = QtGui.QListWidget(self)
        self.membersList.setSelectionMode(QtGui.QAbstractItemView.MultiSelection)

        self.membersList.setViewMode(QtGui.QListView.IconMode)
        self.membersList.setMovement(QtGui.QListView.Static)
        self.membersList.setResizeMode(QtGui.QListView.Adjust)
        self.membersList.setGridSize(QtCore.QSize(32, 16))
        self.membersList.setSelectionRectVisible(False)

        memberLayout = QtGui.QFormLayout()
        memberLayout.setLabelAlignment(QtCore.Qt.AlignRight)
        
        self.runpathLabel = QtGui.QLabel("")
        font = self.runpathLabel.font()
        font.setWeight(QtGui.QFont.Bold)
        self.runpathLabel.setFont(font)

        memberLayout.addRow("Runpath:", self.runpathLabel)

        membersCheckPanel = ListCheckPanel(self.membersList)
        #membersCheckPanel.insertWidget(0, QtGui.QLabel("Members"))

        self.simulateFrom = ValidatedTimestepCombo(parent, fromLabel="Start", toLabel="End of history")
        self.simulateTo = ValidatedTimestepCombo(parent, fromLabel="End of history", toLabel="End of prediction")

        self.startState = QtGui.QComboBox(self)
        self.startState.setMaximumWidth(100)
        self.startState.setToolTip("Select state")
        self.startState.addItem("Analyzed")
        self.startState.addItem("Forecast")

        startLayout = QtGui.QHBoxLayout()
        startLayout.addWidget(self.simulateFrom)
        startLayout.addWidget(self.startState)

        memberLayout.addRow("Run simulation from: ", startLayout)
        memberLayout.addRow("Run simulation to: ", self.simulateTo)
        memberLayout.addRow("Mode: ", self.createRadioButtons())
        memberLayout.addRow(membersCheckPanel)
        memberLayout.addRow("Members:", self.membersList)

        self.actionButton = QtGui.QPushButton("Run simulations")

        self.connect(self.actionButton, QtCore.SIGNAL('clicked()'), self.run)

        actionLayout = QtGui.QHBoxLayout()
        actionLayout.addStretch(1)
        actionLayout.addWidget(self.actionButton)
        actionLayout.addStretch(1)

        memberLayout.addRow(createSpace(10))
        memberLayout.addRow(actionLayout)

        self.setRunpath("...")

        return memberLayout


    def run(self):
        """Do pre run checks and start the simulation. A new dialog will be shown with simulation information."""
        ert = self.getModel()
        selectedMembers = getItemsFromList(self.membersList)

        selectedMembers = [int(member) for member in selectedMembers]

        if len(selectedMembers) == 0:
            QtGui.QMessageBox.warning(self, "Missing data", "At least one member must be selected!")
            return

        if not ert.main.is_initialized:
            QtGui.QMessageBox.warning(self, "Case not initialized", "The case must be initialized before simulation can start!")
            return


        simFrom = self.simulateFrom.getSelectedValue()
        simTo = self.simulateTo.getSelectedValue()

        if self.rbAssimilation.isChecked():
            mode = self.run_mode_type["ENKF_ASSIMILATION"]
        if self.rbExperiment.isChecked():
            if simTo == -1: # -1 == End
                mode = self.run_mode_type["ENSEMBLE_PREDICTION"]
            else:
                mode = self.run_mode_type["ENSEMBLE_EXPERIMENT"]
        if self.rbSmoother.isChecked():
            mode = self.run_mode_type["SMOOTHER"]

        state = self.state_enum["ANALYZED"]
        if self.startState.currentText() == "Forecast" and not simFrom == 0:
            state = self.state_enum["FORECAST"]

        init_step_parameter = simFrom

        #if mode == run_mode_type["ENKF_ASSIMILATION"]:
        #    init_step_parameter = simFrom
        #elif mode == run_mode_type["ENSEMBLE_EXPERIMENT"]:
        #    init_step_parameter = 0
        #else:
        #    init_step_parameter = self.historyLength

        jobsdialog = SimulationsDialog(self)
        jobsdialog.start(ert = ert,
                         memberCount = self.membersList.count(),
                         selectedMembers = selectedMembers,
                         simFrom = simFrom,
                         simTo = simTo,
                         mode = mode,
                         state = state,
                         init_step_parameter = init_step_parameter)
        

    def setRunpath(self, runpath):
        """Update the label widget with a new runpath"""
        self.runpathLabel.setText(runpath)

    def fetchContent(self):
        """Fetch updated data from ERT"""
        data = self.getFromModel()

        self.historyLength = data["history_length"]

        self.membersList.clear()

        for member in data["members"]:
            self.membersList.addItem("%03d" % (member))
        #self.membersList.addItem(str(member))

        self.setRunpath(data["runpath"])

        self.simulateFrom.setHistoryLength(self.historyLength)
        self.simulateTo.setFromValue(self.historyLength)
        self.simulateTo.setToValue(-1)
        self.simulateTo.setMinTimeStep(0)
        self.simulateTo.setMaxTimeStep(self.historyLength)

        self.membersList.selectAll()



    def getter(self, ert):
        """Fetch data from EnKF. Such as number of realizations, runpath and number of timesteps."""
        members = ert.main.ens_size
        historyLength = ert.main.get_history_length
        runpath = ert.main.model_config.get_runpath_as_char

        return {"members" : range(members), "history_length" : historyLength, "runpath" : runpath}


    def rbToggle(self):
        """Activated when a toggle is selected. Enables/disables member selection."""
        if self.rbAssimilation.isChecked():
            self.membersList.setSelectionEnabled(False)
            self.membersList.selectAll()
        else:
            self.membersList.setSelectionEnabled(True)

    def createRadioButtons(self):
        """Create a toggle between assimilation and experiment."""
        radioLayout = QtGui.QVBoxLayout()
        radioLayout.setSpacing(3)
        self.rbExperiment = QtGui.QRadioButton("Ensemble experiment")
        radioLayout.addWidget(self.rbExperiment)
        self.rbAssimilation = QtGui.QRadioButton("EnKF assimilation")
        radioLayout.addWidget(self.rbAssimilation)
        self.rbSmoother = QtGui.QRadioButton("Smoother")
        radioLayout.addWidget(self.rbSmoother)


        self.connect(self.rbAssimilation , QtCore.SIGNAL('toggled(bool)'), lambda : self.rbToggle())
        self.connect(self.rbExperiment  , QtCore.SIGNAL('toggled(bool)'), lambda : self.rbToggle())
        self.connect(self.rbSmoother  , QtCore.SIGNAL('toggled(bool)'), lambda : self.rbToggle())
        
        return radioLayout


class RunPanel(QtGui.QFrame):
    """A panel that represents data relateed to starting simulation."""
    def __init__(self, parent):
        QtGui.QFrame.__init__(self, parent)
        self.setFrameShape(QtGui.QFrame.Panel)
        self.setFrameShadow(QtGui.QFrame.Raised)

        panelLayout = QtGui.QVBoxLayout()
        self.setLayout(panelLayout)

        #        button = QtGui.QPushButton("Refetch")
        #        self.connect(button, QtCore.SIGNAL('clicked()'), ContentModel.updateObservers)
        #        panelLayout.addWidget(button)

        panelLayout.addWidget(RunWidget())



        





#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'forwardmodelpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from ert_gui.widgets.helpedwidget import HelpedWidget, ContentModel
from ert_gui.widgets.searchablelist import SearchableList
from PyQt4 import QtGui, QtCore
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.validateddialog import ValidatedDialog
import ert_gui.widgets.util
import os
from ert_gui.widgets.util import ValidationInfo
from jobsdialog import EditJobDialog
from ert_gui.widgets.stringbox import StringBox

class ForwardModelPanel(HelpedWidget):
    """
    Widget for adding, removing and editing forward models.
    Only uses the setter and getter hooks.
    The panel expects remove to return True or False based on the success of the removal.
    """

    def __init__(self, parent=None):
        HelpedWidget.__init__(self, parent, "Forward Model", "config/simulation/forward_model")

        self.forward_model_job = ForwardModelJob("undefined")

        self.createWidgets(parent)

        self.emptyPanel = ert_gui.widgets.util.createEmptyPanel()

        self.pagesWidget = QtGui.QStackedWidget()
        self.pagesWidget.addWidget(self.emptyPanel)
        self.pagesWidget.addWidget(self.forward_model_panel)
        self.addWidget(self.pagesWidget)

        self.addHelpButton()

    def createWidgets(self, parent):
        self.searchableList = SearchableList(parent, list_height=150, list_width=150, ignore_case=True, order_editable=True)
        self.addWidget(self.searchableList)
        self.connect(self.searchableList, QtCore.SIGNAL('currentItemChanged(QListWidgetItem, QListWidgetItem)'),self.changeParameter)
        self.connect(self.searchableList, QtCore.SIGNAL('addItem(QListWidgetItem)'), self.addItem)
        self.connect(self.searchableList, QtCore.SIGNAL('removeItem(QListWidgetItem)'), self.removeItem)
        self.connect(self.searchableList, QtCore.SIGNAL('orderChanged(QListWidgetItem)'), self.forwardModelChanged)


        self.forward_model_panel = ert_gui.widgets.util.createEmptyPanel()

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)


        self.forward_model_args = StringBox(self, "", "config/simulation/forward_model_arguments")
        self.forward_model_args.initialize = ContentModel.emptyInitializer
        self.forward_model_args.setter = self.setArguments
        self.forward_model_args.getter = lambda model: self.forward_model_job.arguments

        layout.addRow("Arguments:", self.forward_model_args)

        layout.addRow(ert_gui.widgets.util.createSpace(20))

        self.help_text = QtGui.QLabel()
        self.help_text.setText("")

        layout.addRow(ert_gui.widgets.util.centeredWidget(self.help_text))

        self.forward_model_panel.setLayout(layout)
        self.modelConnect('jobListChanged()', self.fetchContent)

    def setArguments(self, model, arguments):
        """Set the arguments of the current forward model job."""
        self.forward_model_job.setArguments(arguments)
        self.forwardModelChanged()

    def fetchContent(self):
        """
        Retrieves data from the model and inserts it into the widget.
        Expects a hash containing these two keys: available_jobs and forward_model.
        available_jobs=list of names
        forward_model)list of tuples containing(name, arguments, help_text)
        """
        data = self.getFromModel()

        self.available_jobs = data['available_jobs']

        forward_model = data['forward_model']
        self.searchableList.list.clear()
        for job in forward_model:
            jobitem = QtGui.QListWidgetItem()
            jobitem.setText(job[0])
            forward_model_job = ForwardModelJob(job[0])
            forward_model_job.setArguments(job[1])
            forward_model_job.setHelpText(job[2])

            jobitem.setData(QtCore.Qt.UserRole, forward_model_job)
            jobitem.setToolTip(job[0])
            self.searchableList.list.addItem(jobitem)

    def setForwardModelJob(self, forward_model_job):
        """Set the current visible forward model job"""
        self.forward_model_job = forward_model_job
        self.help_text.setText(forward_model_job.help_text)
        self.forward_model_args.fetchContent()

    def changeParameter(self, current, previous):
        """Switch between forward models. Selection from the list"""
        if current is None:
            self.pagesWidget.setCurrentWidget(self.emptyPanel)
        else:
            self.pagesWidget.setCurrentWidget(self.forward_model_panel)
            self.setForwardModelJob(current.data(QtCore.Qt.UserRole).toPyObject())

    def forwardModelChanged(self):
        """
        Called whenever the forward model is changed. (reordering, adding, removing)
        The data submitted to the updateContent() (from ContentModel) is a list of tuples (name, arguments)
        """
        items = self.searchableList.getItems()
        currentRow = self.searchableList.list.currentRow()
        forward_model = []
        for item in items:
            forward_model_job = item.data(QtCore.Qt.UserRole).toPyObject()
            forward_model.append((forward_model_job.name, forward_model_job.arguments))

        self.updateContent(forward_model)
        self.fetchContent()
        self.searchableList.list.setCurrentRow(currentRow)

    def addToList(self, list, name):
        """Adds a new job to the list"""
        param = QtGui.QListWidgetItem()
        param.setText(name)

        new_job = ForwardModelJob(name)
        param.setData(QtCore.Qt.UserRole, new_job)

        list.addItem(param)
        list.setCurrentItem(param)
        return param

    def addItem(self, list):
        """Called by the add button to insert a new job"""

        pd = ValidatedDialog(self, "New forward model job", "Select a job:", self.available_jobs, True)
        if pd.exec_():
            self.addToList(list, pd.getName())
            self.forwardModelChanged()

    def removeItem(self, list):
        """Called by the remove button to remove a selected job"""
        currentRow = list.currentRow()

        if currentRow >= 0:
            title = "Delete forward model job?"
            msg = "Are you sure you want to delete the job from the forward model?"
            btns = QtGui.QMessageBox.Yes | QtGui.QMessageBox.No
            doDelete = QtGui.QMessageBox.question(self, title, msg, btns)

            if doDelete == QtGui.QMessageBox.Yes:
                list.takeItem(currentRow)
                self.forwardModelChanged()

class ForwardModelJob:
    """Stores the name, arguments and help text of a job."""

    def __init__(self, name, arguments=None, help_text=""):
        self.name = name
        self.setArguments(arguments)
        self.setHelpText(help_text)

    def setArguments(self, args):
        if args is None:
            args = ""
        self.arguments = args

    def setHelpText(self, text):
        if text == "":
            self.help_text = "No help available for this job."
        else:
            self.help_text = text



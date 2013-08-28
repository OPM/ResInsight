#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'jobspanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class JobsPanel(HelpedWidget):
    """
    Widget for adding, removing and editing jobs.
    These additional ContentModel functions must be implemented: insert and remove.
    The panel expects remove to return True or False based on the success of the removal.
    """

    def __init__(self, parent=None):
        HelpedWidget.__init__(self, parent, "", "config/systemenv/install_jobs")

        self.job = Job("undefined")

        self.createWidgets(parent)

        self.emptyPanel = ert_gui.widgets.util.createEmptyPanel()

        self.pagesWidget = QtGui.QStackedWidget()
        self.pagesWidget.addWidget(self.emptyPanel)
        self.pagesWidget.addWidget(self.jobPanel)
        self.addWidget(self.pagesWidget)

    def createWidgets(self, parent):
        self.searchableList = SearchableList(parent, list_height=200, list_width=150, ignore_case=True)
        self.addWidget(self.searchableList)
        self.connect(self.searchableList , QtCore.SIGNAL('currentItemChanged(QListWidgetItem, QListWidgetItem)'),self.changeParameter)
        self.connect(self.searchableList , QtCore.SIGNAL('addItem(QListWidgetItem)'), self.addItem)
        self.connect(self.searchableList , QtCore.SIGNAL('removeItem(QListWidgetItem)'), self.removeItem)


        self.jobPanel = ert_gui.widgets.util.createEmptyPanel()

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)

        self.jobpath = PathChooser(self, "", "config/systemenv/install_job_path", show_files=True, must_be_set=True)
        self.jobpath.initialize = ContentModel.emptyInitializer
        self.jobpath.setter = self.setPath
        self.jobpath.getter = lambda model: self.job.path

        layout.addRow("Job:", self.jobpath)

        layout.addRow(ert_gui.widgets.util.createSpace(20))

        self.validationInfo = ValidationInfo(ValidationInfo.EXCLAMATION)
        self.validationInfo.setMessage("Pressing edit will create a job that does not exist.")

        self.editButton = QtGui.QPushButton(self)
        self.editButton.setToolTip("Edit job")
        self.editButton.setIcon(ert_gui.widgets.util.resourceIcon("cog"))
        self.editButton.setText("Edit")
        self.connect(self.editButton, QtCore.SIGNAL('clicked()'), self.editJob)


        layout.addRow(ert_gui.widgets.util.centeredWidget(self.editButton))

        layout.addRow(ert_gui.widgets.util.centeredWidget(self.validationInfo))


        self.jobPanel.setLayout(layout)

    def setPath(self, model, path):
        self.job.set("path", path)
        self.updateContent(self.job)

#        if os.path.exists(path):
#            self.validationInfo.setMessage("")
#        else:
#            self.validationInfo.setMessage("The path must exist! Edit to create the job.")

    def editJob(self):
        if not os.path.exists(Job.path_prefix):
            os.mkdir(Job.path_prefix)

        ejd = EditJobDialog(self)
        ejd.setJob(self.job)
        ejd.exec_()
        self.jobpath.validatePath()

    def fetchContent(self):
        """Retrieves data from the model and inserts it into the widget"""
        jobs = self.getFromModel()

        for job in jobs:
            jobitem = QtGui.QListWidgetItem()
            jobitem.setText(job.name)
            jobitem.setData(QtCore.Qt.UserRole, job)
            jobitem.setToolTip(job.name)
            self.searchableList.list.addItem(jobitem)

    def setJob(self, job):
        self.job = job
        self.jobpath.fetchContent()

    def changeParameter(self, current, previous):
        """Switch between jobs. Selection from the list"""
        if current is None:
            self.pagesWidget.setCurrentWidget(self.emptyPanel)
        else:
            self.pagesWidget.setCurrentWidget(self.jobPanel)
            self.setJob(current.data(QtCore.Qt.UserRole).toPyObject())

    def addToList(self, list, name):
        """Adds a new job to the list"""
        param = QtGui.QListWidgetItem()
        param.setText(name)

        new_job = Job(name)
        param.setData(QtCore.Qt.UserRole, new_job)

        list.addItem(param)
        list.setCurrentItem(param)
        return new_job

    def addItem(self, list):
        """Called by the add button to insert a new job"""
        uniqueNames = []
        for index in range(list.count()):
            uniqueNames.append(str(list.item(index).text()))

        pd = ValidatedDialog(self, "New job", "Enter name of new job:", uniqueNames)
        if pd.exec_():
            new_job = self.addToList(list, pd.getName())

            self.updateContent(new_job, operation=self.INSERT)
            self.modelEmit('jobListChanged()')

    def removeItem(self, list):
        """Called by the remove button to remove a selected job"""
        currentRow = list.currentRow()

        if currentRow >= 0:
            title = "Delete job?"
            msg = "Are you sure you want to delete the job?"
            btns = QtGui.QMessageBox.Yes | QtGui.QMessageBox.No
            doDelete = QtGui.QMessageBox.question(self, title, msg, btns)

            if doDelete == QtGui.QMessageBox.Yes:
                item = list.currentItem()
                job = item.data(QtCore.Qt.UserRole).toPyObject()
                success = self.updateContent(job, operation=self.REMOVE)
                if success:
                    list.takeItem(currentRow)
                    self.modelEmit('jobListChanged()')


class Job:
    path_prefix = "private_jobs"

    def __init__(self, name, path=None):
        self.name = name

        if path is None:
            self.path = self.path_prefix + "/" + name
        else:
            self.path = str(path)


    def set(self, attr, value):
        setattr(self, attr, value)

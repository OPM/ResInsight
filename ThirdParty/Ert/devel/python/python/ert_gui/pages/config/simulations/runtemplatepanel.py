#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'runtemplatepanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert_gui.widgets.searchablelist import SearchableList
from PyQt4 import QtGui, QtCore
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.validateddialog import ValidatedDialog
import ert_gui.widgets.util
import os
from ert_gui.widgets.util import ValidationInfo
from ert_gui.pages.config.jobs.jobsdialog import EditJobDialog
from ert_gui.widgets.stringbox import StringBox
from ert_gui.widgets.helpedwidget import ContentModel

class RunTemplatePanel(HelpedWidget):
    """
    Widget for adding, removing and editing run templates.
    """

    def __init__(self, parent=None):
        HelpedWidget.__init__(self, parent, "", "config/simulation/run_template")

        self.run_template = RunTemplate("undefined", "", "", "")

        self.createWidgets(parent)

        self.emptyPanel = ert_gui.widgets.util.createEmptyPanel()

        self.pagesWidget = QtGui.QStackedWidget()
        self.pagesWidget.addWidget(self.emptyPanel)
        self.pagesWidget.addWidget(self.run_template_panel)
        self.addWidget(self.pagesWidget)

        self.addHelpButton()

    def createWidgets(self, parent):
        self.searchableList = SearchableList(parent, list_height=150, list_width=150, ignore_case=True)
        self.addWidget(self.searchableList)
        self.connect(self.searchableList, QtCore.SIGNAL('currentItemChanged(QListWidgetItem, QListWidgetItem)'),
                     self.changeParameter)
        self.connect(self.searchableList, QtCore.SIGNAL('addItem(QListWidgetItem)'), self.addItem)
        self.connect(self.searchableList, QtCore.SIGNAL('removeItem(QListWidgetItem)'), self.removeItem)


        self.run_template_panel = ert_gui.widgets.util.createEmptyPanel()

        layout = QtGui.QFormLayout()
        layout.setLabelAlignment(QtCore.Qt.AlignRight)

        self.run_template_file = PathChooser(self, "", "config/simulation/run_template_file", show_files=True)
        self.run_template_file.initialize = ContentModel.emptyInitializer
        self.run_template_file.setter = self.setTemplateFile
        self.run_template_file.getter = lambda model: self.run_template.template_file

        self.run_template_target = PathChooser(self, "", "config/simulation/run_template_target", show_files=True)
        self.run_template_target.initialize = ContentModel.emptyInitializer
        self.run_template_target.setter = self.setTargetFile
        self.run_template_target.getter = lambda model: self.run_template.target_file

        self.run_template_args = StringBox(self, "", "config/simulation/run_template_arguments")
        self.run_template_args.initialize = ContentModel.emptyInitializer
        self.run_template_args.setter = self.setArguments
        self.run_template_args.getter = lambda model: self.run_template.arguments

        layout.addRow("Template file:", self.run_template_file)
        layout.addRow("Target file:", self.run_template_target)
        layout.addRow("Arguments:", self.run_template_args)

        layout.addRow(ert_gui.widgets.util.createSpace(20))

        self.run_template_panel.setLayout(layout)

    def setArguments(self, model, arguments):
        self.run_template.setArguments(arguments)
        self.runTemplateChanged()

    def setTemplateFile(self, model, template_file):
        self.run_template.setTemplateFile(template_file)
        self.runTemplateChanged()

    def setTargetFile(self, model, target_file):
        self.run_template.setTargetFile(target_file)
        self.runTemplateChanged()


    def fetchContent(self):
        """
        Retrieves data from the model and inserts it into the widget.
        List of tuples: (name, template_file, target_file, arguments)
        """
        data = self.getFromModel()

        self.searchableList.list.clear()
        for item in data:
            jobitem = QtGui.QListWidgetItem()
            jobitem.setText(item[0])
            run_template = RunTemplate(item[0], item[1], item[2], item[3])

            jobitem.setData(QtCore.Qt.UserRole, run_template)
            jobitem.setToolTip(item[0])
            self.searchableList.list.addItem(jobitem)

    def setRunTemplate(self, run_template):
        """Set the current visible run template"""
        self.run_template = run_template
        self.run_template_args.fetchContent()
        self.run_template_file.fetchContent()
        self.run_template_target.fetchContent()

    def changeParameter(self, current, previous):
        """Switch between run templates. Selection from the list"""
        if current is None:
            self.pagesWidget.setCurrentWidget(self.emptyPanel)
        else:
            self.pagesWidget.setCurrentWidget(self.run_template_panel)
            self.setRunTemplate(current.data(QtCore.Qt.UserRole).toPyObject())

    def runTemplateChanged(self):
        """
        Called whenever the run template is changed. (adding, removing)
        The data submitted to the updateContent() (from ContentModel) is a list of tuples (name, template_file, target_file, arguments)
        """
        items = self.searchableList.getItems()
        currentRow = self.searchableList.list.currentRow()
        run_templates = []
        for item in items:
            r_t = item.data(QtCore.Qt.UserRole).toPyObject()
            run_template_tuple = (r_t.name, r_t.template_file, r_t.target_file, r_t.arguments)
            run_templates.append(run_template_tuple)

        self.updateContent(run_templates)
        self.fetchContent()
        self.searchableList.list.setCurrentRow(currentRow)

    def addToList(self, list, name):
        """Adds a new run template to the list"""
        param = QtGui.QListWidgetItem()
        param.setText(name)

        new_job = RunTemplate(name)
        param.setData(QtCore.Qt.UserRole, new_job)

        list.addItem(param)
        list.setCurrentItem(param)
        return param

    def addItem(self, list):
        """Called by the add button to insert a new run template"""
        uniqueNames = []
        for index in range(list.count()):
            uniqueNames.append(str(list.item(index).text()))

        pd = ValidatedDialog(self, "New run template", "Enter name of new run template:", uniqueNames)
        if pd.exec_():
            self.addToList(list, pd.getName())
            self.runTemplateChanged()

    def removeItem(self, list):
        """Called by the remove button to remove a selected job"""
        currentRow = list.currentRow()

        if currentRow >= 0:
            title = "Delete run template?"
            msg = "Are you sure you want to delete the run template?"
            btns = QtGui.QMessageBox.Yes | QtGui.QMessageBox.No
            doDelete = QtGui.QMessageBox.question(self, title, msg, btns)

            if doDelete == QtGui.QMessageBox.Yes:
                list.takeItem(currentRow)
                self.runTemplateChanged()

class RunTemplate:
    """Stores the name, arguments and help text of a run template."""

    def __init__(self, name, template_file="", target_file="", arguments=""):
        self.name = name
        self.setArguments(arguments)
        self.setTargetFile(target_file)
        self.setTemplateFile(template_file)

    def setArguments(self, args):
        if args is None:
            args = ""
        self.arguments = args

    def setTemplateFile(self, template_file):
        if template_file is None:
            template_file = ""
        self.template_file = template_file

    def setTargetFile(self, target_file):
        if target_file is None:
            target_file = ""
        self.target_file = target_file





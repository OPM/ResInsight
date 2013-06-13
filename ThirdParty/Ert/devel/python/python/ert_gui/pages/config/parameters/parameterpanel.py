#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'parameterpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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

from ert_gui.widgets.helpedwidget import *
from ert_gui.widgets.tablewidgets import AddRemoveWidget
from ert_gui.widgets import util
from ert_gui.widgets.pathchooser import PathChooser
from ert_gui.widgets.combochoice import ComboChoice
import ert_gui.widgets.stringbox
from fieldpanel import *
from parameterdialog import ParameterDialog
from ert_gui.widgets.searchablelist import SearchableList
from datapanel import DataPanel
from keywordpanel import KeywordPanel
import ert_gui.widgets.util
from parametermodels import SummaryModel, FieldModel, DataModel, KeywordModel
from PyQt4.QtCore import SIGNAL


class ParameterPanel(HelpedWidget):
    """Shows a widget for parameters. The data structure expected and sent to the getter and setter is an array of Parameters."""

    def __init__(self, parent=None, label="", help=""):
        """Construct a ParameterPanel."""
        HelpedWidget.__init__(self, parent, label, help)

        self.searchableList = SearchableList(converter=lambda item : item.getName(), list_width=175)
        self.addWidget(self.searchableList)


        self.pagesWidget = QtGui.QStackedWidget()

        self.emptyPanel = ert_gui.widgets.util.createEmptyPanel()

        self.fieldPanel = FieldPanel(self)
        self.dataPanel = DataPanel(self)
        self.keywordPanel = KeywordPanel(self)

        self.pagesWidget.addWidget(self.emptyPanel)
        self.pagesWidget.addWidget(self.fieldPanel)
        self.pagesWidget.addWidget(self.dataPanel)
        self.pagesWidget.addWidget(self.keywordPanel)

        self.addWidget(self.pagesWidget)

        self.connect(self.searchableList, QtCore.SIGNAL('currentItemChanged(QListWidgetItem, QListWidgetItem)'), self.changeParameter)
        self.connect(self.searchableList, QtCore.SIGNAL('addItem(QListWidgetItem)'), self.addItem)
        self.connect(self.searchableList, QtCore.SIGNAL('removeItem(QListWidgetItem)'), self.removeItem)

        #self.addHelpButton()


    def changeParameter(self, current, previous):
        if not current:
            self.pagesWidget.setCurrentWidget(self.emptyPanel)
        elif FieldModel.TYPE == current.getType():
            self.pagesWidget.setCurrentWidget(self.fieldPanel)
            self.fieldPanel.setFieldModel(current.getUserData())
        elif DataModel.TYPE == current.getType():
            self.pagesWidget.setCurrentWidget(self.dataPanel)
            self.dataPanel.setDataModel(current.getUserData())
        elif KeywordModel.TYPE == current.getType():
            self.pagesWidget.setCurrentWidget(self.keywordPanel)
            self.keywordPanel.setKeywordModel(current.getUserData())
        else:
            self.pagesWidget.setCurrentWidget(self.emptyPanel)


    def createParameter(self, type_name, name):
        """Adds a new parameter to the list"""

        if type_name == FieldModel.TYPE.name:
            type = FieldModel.TYPE
            data = FieldModel(name)
        elif type_name == DataModel.TYPE.name:
            type = DataModel.TYPE
            data = DataModel(name)
        elif type_name == KeywordModel.TYPE.name:
            type = KeywordModel.TYPE
            data = KeywordModel(name)
        elif type_name == SummaryModel.TYPE.name:
            type = SummaryModel.TYPE
            data = SummaryModel(name)
        else:
            raise AssertionError("Type name unknown: %s" % (type_name))

        param = Parameter(name, type)
        param.setUserData(data)
        param.setValid(False)
        return param


    def addToList(self, list, parameter):
        list.addItem(parameter)
        list.setCurrentItem(parameter)

        user_data = parameter.getUserData()
        ##self.connect(user_data, SIGNAL('modelChanged(Model)'), self.modelChanged)

    def modelChanged(self, parameter_model):
        """Called whenever the content of a model changes"""
        self.updateContent(parameter_model)


    def addItem(self, list):
        """Called by the add button to insert a new parameter. A Parameter object is sent to the ContentModel inserter"""
        uniqueNames = []
        for index in range(list.count()):
            uniqueNames.append(str(list.item(index).text()))

        pd = ParameterDialog(self, Parameter.get_typeIcons() , uniqueNames)
        if pd.exec_():
            parameter = self.createParameter(pd.getTypeName(), pd.getName())
            ok = self.updateContent(parameter, operation=ContentModel.INSERT)
            if ok:
                self.addToList(list, parameter)

        # todo: emit when a new field is added also make initandcopy listen -> self.modelEmit("casesUpdated()")


    def removeItem(self, list):
        """Called by the remove button to remove a selected parameter. The key is forwarded to the ContentModel remover"""
        currentRow = list.currentRow()

        if currentRow >= 0:
            doDelete = QtGui.QMessageBox.question(self, "Delete parameter?", "Are you sure you want to delete the parameter?", QtGui.QMessageBox.Yes | QtGui.QMessageBox.No)

            if doDelete == QtGui.QMessageBox.Yes:
                item = list.item(currentRow)
                user_data = item.getUserData()
                self.disconnect(user_data, SIGNAL('modelChanged(Model)'), self.modelChanged)
                self.updateContent(item.getName(), operation=ContentModel.REMOVE)
                list.takeItem(currentRow)
        #todo: emit change

    def fetchContent(self):
        """Retrieves data from the model and inserts it into the list"""
        parameters = self.getFromModel()

        for parameter in parameters:
            if parameter is None:
                sys.stderr.write("Unknown type name!\n")
                break
                #raise AssertionError("Unknown type name!")

            param = Parameter(parameter.name, parameter.TYPE)
            param.setUserData(parameter)
            param.setValid(parameter.isValid())

            self.addToList(self.searchableList.getList(), param)

        if self.searchableList.getList().count > 0:
            self.searchableList.getList().setCurrentRow(0)



class Parameter(QtGui.QListWidgetItem):
    """ListWidgetItem class that represents a Parameter with an associated icon."""

    typeIcons__ = None

    @classmethod
    def get_typeIcons(cls):
        if cls.typeIcons__ is None:
            typeIcons__ = {FieldModel.TYPE: util.resourceIcon("grid_16"),
                           DataModel.TYPE: util.resourceIcon("data"),
                           SummaryModel.TYPE: util.resourceIcon("summary"),
                           KeywordModel.TYPE: util.resourceIcon("key")}
        return typeIcons__
        


    def __init__(self, name, type, icon=None):
        if icon is None:
            icon = Parameter.get_typeIcons()[type]
            
        QtGui.QListWidgetItem.__init__(self, icon, name)
        self.type = type
        self.name = name
        self.user_data = None
        self.setValid(True)
    
    def getType(self):
        """Retruns the type of this parameter"""
        return self.type
    
    def getName(self):
        """Returns the name of this parameter (keyword)"""
        return self.name
    
    def __ge__(self, other):
        if self.type.name == other.type.name:
            return self.name.lower() >= other.name.lower()
        else:
            return self.type.name >= other.type.name
    
    def __lt__(self, other):
        return not self >= other
    
    def setUserData(self, data):
        """Set user data for this parameter."""
        self.user_data = data
    
    def getUserData(self):
        """Retrieve the user data."""
        return self.user_data
    
    def setValid(self, valid):
        """Set the validity of this item. An invalid item is colored red"""
        self.valid = valid
    
        if valid:
            self.setBackgroundColor(QtCore.Qt.white)
        else:
            self.setBackgroundColor(HelpedWidget.STRONG_ERROR_COLOR)



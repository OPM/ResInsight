#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'searchablelist.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from tablewidgets import AddRemoveWidget
from PyQt4 import QtGui, QtCore
from tablewidgets import OrderWidget

class SearchableList(QtGui.QWidget):
    """
    A searchable list of items.
    Emits addItem(QListWidget) and removeItem(QListWidget) when the add and remove buttons are pressed.
    """
    passiveColor = QtGui.QColor(194, 194, 194)

    def __init__(self, parent=None, converter=lambda item : str(item.text()), list_height=350, list_width = 130, ignore_case=False, order_editable=False):
        QtGui.QWidget.__init__(self, parent)
        self.setMaximumWidth(list_width)
        self.setMinimumWidth(list_width)
        self.converter = converter
        self.ignore_case = ignore_case

        vlayout = QtGui.QVBoxLayout()
        vlayout.setMargin(0)

        self.searchBox = QtGui.QLineEdit(parent)
        self.searchBox.setToolTip("Type to search!")
        self.searchBox.focusInEvent = lambda event : self.enterSearch(event)
        self.searchBox.focusOutEvent = lambda event : self.exitSearch(event)
        self.activeColor = self.searchBox.palette().color(self.searchBox.foregroundRole())
        self.disableSearch = True
        self.presentSearch()
        self.connect(self.searchBox, QtCore.SIGNAL('textChanged(QString)'), self.searchInList)
        vlayout.addWidget(self.searchBox)

        self.list = QtGui.QListWidget(parent)
        self.list.setMaximumWidth(list_width - 2)
        self.list.setMinimumWidth(list_width - 2)
        self.list.setMinimumHeight(list_height)

        if not order_editable:
            self.list.setSortingEnabled(True)
            
        vlayout.addWidget(self.list)

        addItem = lambda : self.emit(QtCore.SIGNAL("addItem(list)"), self.list)
        removeItem = lambda : self.emit(QtCore.SIGNAL("removeItem(list)"), self.list)
        add_remove_widget = AddRemoveWidget(self, addItem, removeItem, True)

        if order_editable:
            def moveItemUp():
                index = self.list.currentRow()
                if index > 0 and self.list.count() > 1:
                    item = self.list.takeItem(index)
                    self.list.insertItem(index - 1, item)
                    self.list.setCurrentItem(item)
                    self.emit(QtCore.SIGNAL("orderChanged(list)"), self.list)

            def moveItemDown():
                index = self.list.currentRow()
                if index < self.list.count() - 1 and self.list.count() > 1:
                    item = self.list.takeItem(index)
                    self.list.insertItem(index + 1, item)
                    self.list.setCurrentItem(item)
                    self.emit(QtCore.SIGNAL("orderChanged(list)"), self.list)

            hlayout = QtGui.QHBoxLayout()
            hlayout.setMargin(0)
            hlayout.addWidget(OrderWidget(self, moveItemUp, moveItemDown, True))
            
            hlayout.addWidget(add_remove_widget)
            vlayout.addLayout(hlayout)

        else:
            vlayout.addWidget(add_remove_widget)



            
        self.setLayout(vlayout)

        def emitter(current, previous):
            self.emit(QtCore.SIGNAL("currentItemChanged(QListWidgetItem, QListWidgetItem)"), current, previous)

        self.connect(self.list, QtCore.SIGNAL('currentItemChanged(QListWidgetItem *, QListWidgetItem *)'), emitter)

    def presentSearch(self):
        """Is called to present the greyed out search"""
        self.disableSearch = True
        self.searchBox.setText("Search")
        palette = self.searchBox.palette()
        palette.setColor(self.searchBox.foregroundRole(), self.passiveColor)
        self.searchBox.setPalette(palette)

    def activateSearch(self):
        """Is called to remove the greyed out search"""
        self.disableSearch = False
        self.searchBox.setText("")
        palette = self.searchBox.palette()
        palette.setColor(self.searchBox.foregroundRole(), self.activeColor)
        self.searchBox.setPalette(palette)

    def enterSearch(self, focusEvent):
        """Called when the line edit gets the focus"""
        QtGui.QLineEdit.focusInEvent(self.searchBox, focusEvent)
        if str(self.searchBox.text()) == "Search":
            self.activateSearch()

    def exitSearch(self, focusEvent):
        """Called when the line edit looses focus"""
        QtGui.QLineEdit.focusOutEvent(self.searchBox, focusEvent)
        if str(self.searchBox.text()) == "":
            self.presentSearch()

    def searchInList(self, value):
        """Called when the contents of the search box changes"""
        if not self.disableSearch:
            for index in range(self.list.count()):
                item = self.list.item(index)
                text = self.converter(item)

                if self.ignore_case:
                    value = str(value).lower()
                    text = text.lower()

                if not text.find(value) == -1:
                    item.setHidden(False)
                else:
                    item.setHidden(True)

    def getList(self):
        """Returns the contained list widget"""
        return self.list

    def getItems(self):
        items = []

        for index in range(self.list.count()):
            items.append(self.list.item(index))
        return items

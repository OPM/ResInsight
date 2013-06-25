#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'runpathpanel.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from ert.ert.enums import keep_runpath_type
from ert_gui.widgets.helpedwidget import HelpedWidget
from ert_gui.pages.run.legend import Legend

class RunpathMemberList(QtGui.QListWidget):
    """A list widget with custom items representing members"""
    def __init__(self):
        QtGui.QListWidget.__init__(self)

        self.setViewMode(QtGui.QListView.IconMode)
        self.setMovement(QtGui.QListView.Static)
        self.setResizeMode(QtGui.QListView.Adjust)

        self.setItemDelegate(RunpathMemberItemDelegate())
        self.setSelectionMode(QtGui.QAbstractItemView.ExtendedSelection)
        self.setSelectionRectVisible(False)

        self.setSortingEnabled(True)
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)


class RunpathMemberItem(QtGui.QListWidgetItem):
    """Items for the custom SimulationList"""
    def __init__(self, member, runpath_state):
        self.runpath_state = runpath_state
        self.member = member
        QtGui.QListWidgetItem.__init__(self)
        self.setData(QtCore.Qt.DisplayRole, (member, runpath_state))

    def __ge__(self, other):
        return self.member >= other.member

    def __lt__(self, other):
        return not self >= other

    def setState(self, state):
        self.runpath_state = state
        self.setData(QtCore.Qt.DisplayRole, (self.member, self.runpath_state))


class RunpathMemberItemDelegate(QtGui.QStyledItemDelegate):
    """The delegate that renders the custom RunpathMemberItems"""

    default  = QtGui.QColor(255, 255, 240)
    delete  = QtGui.QColor(255, 200, 200)
    keep = QtGui.QColor(200, 255, 200)
    unknown = QtGui.QColor(255, 200, 64)

    size = QtCore.QSize(32, 18)

    def __init__(self):
        QtGui.QStyledItemDelegate.__init__(self)

    def paint(self, painter, option, index):
        """Renders the item"""
        painter.save()
        painter.setRenderHint(QtGui.QPainter.Antialiasing)

        data = index.data(QtCore.Qt.DisplayRole)

        if data is None:
            data = (0, keep_runpath_type.DEFAULT_KEEP)
        else:
            data = data.toPyObject()

        if data[1] == keep_runpath_type.DEFAULT_KEEP:
            color = self.default
        elif data[1] == keep_runpath_type.EXPLICIT_KEEP:
            color = self.keep
        elif data[1] == keep_runpath_type.EXPLICIT_DELETE:
            color = self.delete
        else:
            color = self.unknown

        painter.setPen(color)
        rect = QtCore.QRect(option.rect)
        rect.setX(rect.x() + 1)
        rect.setY(rect.y() + 1)
        rect.setWidth(rect.width() - 2)
        rect.setHeight(rect.height() - 2)
        painter.fillRect(rect, color)

        painter.setPen(QtCore.Qt.black)

        painter.setRenderHint(QtGui.QPainter.Antialiasing, False)
        painter.drawRect(rect)

        if option.state & QtGui.QStyle.State_Selected:
            painter.fillRect(option.rect, QtGui.QColor(255, 255, 255, 150))

        painter.drawText(rect, QtCore.Qt.AlignCenter + QtCore.Qt.AlignVCenter, str(data[0]))

        painter.restore()

    def sizeHint(self, option, index):
        """Returns the size of the item"""
        return self.size


class RunpathMemberPanel(HelpedWidget):
    """A dialog that shows the progress of a simulation"""
    def __init__(self, parent=None, widgetLabel="", helpLabel=""):
        HelpedWidget.__init__(self, widgetLabel=widgetLabel, helpLabel=helpLabel)

        layout = QtGui.QVBoxLayout()
        self.runpath_member_list = RunpathMemberList()
        self.runpath_member_list.contextMenuEvent = self._contextMenu
        layout.addWidget(self.runpath_member_list)
        #self.addWidget(self.runpath_member_list)
        #self.connect(self.runpath_member_list, QtCore.SIGNAL('itemSelectionChanged()'), self.ctrl.selectSimulation)

        legendLayout = QtGui.QHBoxLayout()
        legendLayout.addLayout(Legend("Default", RunpathMemberItemDelegate.default))
        legendLayout.addLayout(Legend("Keep", RunpathMemberItemDelegate.keep))
        legendLayout.addLayout(Legend("Delete", RunpathMemberItemDelegate.delete))
        layout.addLayout(legendLayout)
        self.addLayout(layout)

        self.addHelpButton()


    def _createAction(self, name, func, parent=None):
        """Create an action for the right click menu"""
        action = QtGui.QAction(name, parent)
        action.connect(action, QtCore.SIGNAL("triggered()"), func)
        return action

    def _contextMenu(self, event):
        """Create a right click menu for the simulation view."""
        menu = QtGui.QMenu(self.runpath_member_list)
        selectAll = self._createAction("Select all", self.runpath_member_list.selectAll)
        unselectAll = self._createAction("Unselect all", self.runpath_member_list.clearSelection)
        defaultSelected = self._createAction("Default", lambda : self.setState(keep_runpath_type.DEFAULT_KEEP))
        keepSelected = self._createAction("Keep", lambda : self.setState(keep_runpath_type.EXPLICIT_KEEP))
        deleteSelected = self._createAction("Delete", lambda : self.setState(keep_runpath_type.EXPLICIT_DELETE))

        menu.addAction(defaultSelected)
        menu.addAction(keepSelected)
        menu.addAction(deleteSelected)
        menu.addSeparator()
        menu.addAction(selectAll)
        menu.addAction(unselectAll)
        menu.exec_(event.globalPos())

    def fetchContent(self):
        data = self.getFromModel()
        self.runpath_member_list.clear()

        for item in data:
            self.runpath_member_list.addItem(RunpathMemberItem(item[0], item[1]))            

    def setState(self, state):
        items = self.runpath_member_list.selectedItems()
        for item in items:
            item.setState(state)

        self.updateContent(items)

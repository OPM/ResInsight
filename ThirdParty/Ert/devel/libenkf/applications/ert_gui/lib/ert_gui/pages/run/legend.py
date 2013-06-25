#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'legend.py' is part of ERT - Ensemble based Reservoir Tool. 
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

class LegendMarker(QtGui.QWidget):
    """A widget that shows a colored box"""
    def __init__(self, color, parent = None):
        QtGui.QWidget.__init__(self, parent)

        self.setMaximumSize(QtCore.QSize(12, 12))
        self.setMinimumSize(QtCore.QSize(12, 12))

        self.color = color

    def paintEvent(self, paintevent):
        """Paints the box"""
        painter = QtGui.QPainter(self)

        rect = self.contentsRect()
        rect.setWidth(rect.width() - 1)
        rect.setHeight(rect.height() - 1)
        painter.drawRect(rect)

        rect.setX(rect.x() + 1)
        rect.setY(rect.y() + 1)
        painter.fillRect(rect, self.color)

class Legend(QtGui.QHBoxLayout):
    """Combines a LegendMarker with a label"""
    def __init__(self, legend, color, parent=None):
        QtGui.QHBoxLayout.__init__(self, parent)

        legendMarker = LegendMarker(color, parent)
        self.addWidget(legendMarker)
        self.addWidget(QtGui.QLabel(legend))
        legendMarker.setToolTip(legend)
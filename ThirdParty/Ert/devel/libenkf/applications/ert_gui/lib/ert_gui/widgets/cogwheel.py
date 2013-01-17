#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'cogwheel.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from util import frange
import math

class Cogwheel(QtGui.QWidget):
    """A rotating cogwheel that indicates that a process is running."""

    def __init__(self, color=QtGui.QColor(128, 128, 128), size=64, parent = None):
        QtGui.QWidget.__init__(self, parent)

        self.size = size
        qsize = QtCore.QSize(size, size)
        self.setMaximumSize(qsize)
        self.setMinimumSize(qsize)

        self.color = color
        self.inc = 0
        self.step = 2.5

        self._createCogwheel(size)

        timer = QtCore.QTimer(self)
        self.connect(timer, QtCore.SIGNAL("timeout()"), self, QtCore.SLOT("update()"))
        timer.start(16)

        self.running = False


    def paintEvent(self, paintevent):
        """Called whenever the widget needs to redraw"""
        painter = QtGui.QPainter(self)
        painter.setRenderHint(QtGui.QPainter.Antialiasing)

        rect = self.contentsRect()

        painter.setPen(QtGui.QColor(255, 255, 255, 0))

        painter.setClipRect(rect)
        painter.translate(rect.center())
        painter.rotate(self.step * self.inc)
        self._drawCogwheel(painter)

        r = (self.size / 2.0) * 0.3
        painter.setBrush(QtGui.QBrush(self.color.light(150)))
        painter.drawEllipse(QtCore.QPointF(0, 0), r, r)

        if self.running:
            self.inc += 1


    def _drawCogwheel(self, painter):
        """Draw the cogwheel polygon"""
        painter.save()
        painter.setBrush(QtGui.QBrush(self.color))
        painter.drawPolygon(QtGui.QPolygonF(self.points), len(self.points))
        painter.restore()


    def _createCogwheel(self, size):
        """Creates the points that are part of the cogwheel polygon"""
        self.points = []
        r1 = (size / 2.0) - 1.0
        r2 = 0.80
        teeth = 9
        out = False
        for t in frange(0.0, 2 * math.pi, 2 * math.pi / (teeth * 2.0)):
            x = r1 * math.cos(t)
            y = r1 * math.sin(t)
            if out:
                self.points.append(QtCore.QPointF(x, y))
                self.points.append(QtCore.QPointF(r2 * x, r2 * y))
            else:
                self.points.append(QtCore.QPointF(r2 * x, r2 * y))
                self.points.append(QtCore.QPointF(x, y))
            out = not out

    def setRunning(self, bool):
        """Set wether it should animat or not"""
        self.running = bool

    def isRunning(self):
        return self.running

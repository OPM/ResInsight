#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'zoomslider.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4.QtGui import QFrame, QPainter, QColor
from PyQt4.QtCore import QRectF, SIGNAL
from PyQt4.Qt import QApplication, Qt

class ZoomSlider(QFrame):
    """
    Two way slider representing narrowing of a view.
    The sliders coorespond to factors: a min value and a max value in the range [0, 1]
    Emits zoomValueChanged(float, float) whenever the markers are adjusted. (float, float) -> (min, max)
    """
    def __init__(self, parent=None, horizontal=True):
        QFrame.__init__(self, parent)

        self.horizontal = horizontal
        if horizontal:
            self.setFrameShape(QFrame.HLine)
            self.setMinimumHeight(21)
            self.tilt = 90
        else:
            self.setFrameShape(QFrame.VLine)
            self.setMinimumWidth(21)
            self.tilt = 180

        self.setFrameShadow(QFrame.Sunken)
        self.setMidLineWidth(3)

        self.setMouseTracking(True)

        self.size = 12

        self.min_value = 0.0
        self.max_value = 1.0

        self.setDefaultColors()
        self.button = Qt.NoButton
        self.selected_marker = 'none'


    def paintEvent(self, paint_event):
        QFrame.paintEvent(self, paint_event)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        w = self.width()
        h = self.height()

        if self.horizontal:
            self.min_marker = QRectF(w * self.min_value, 4, self.size, self.size)
            self.max_marker = QRectF(w * self.max_value - self.size - 1, 4, self.size, self.size)
        else:
            self.min_marker = QRectF(4, h - h * self.min_value - self.size - 1, self.size, self.size)
            self.max_marker = QRectF(4, h - h * self.max_value, self.size, self.size)

        pen = painter.pen()
        pen.setWidth(0)
        pen.setColor(QApplication.palette().background().color().dark())
        painter.setPen(pen)

        painter.setBrush(self.min_marker_brush)
        painter.drawPie(self.min_marker, self.tilt * 16, 180 * 16)

        painter.setBrush(self.max_marker_brush)
        painter.drawPie(self.max_marker, self.tilt * 16, -180 * 16)

    def resizeEvent (self, resize_event):
        QFrame.resizeEvent(self, resize_event)


    def _getMinTestMarker(self):
        """Returns the "real" marker bounds. Adjusted for the missing part of an arc."""
        if self.horizontal:
            return QRectF(self.min_marker.left(),
                          self.min_marker.top(),
                          self.min_marker.width() / 2.0,
                          self.min_marker.height())
        else:
            return QRectF(self.min_marker.left(),
                          self.min_marker.top() + self.min_marker.height() / 2.0,
                          self.min_marker.width(),
                          self.min_marker.height() / 2.0)

    def _getMaxTestMarker(self):
        """Returns the "real" marker bounds. Adjusted for the missing part of an arc."""
        if self.horizontal:
            return QRectF(self.max_marker.left() + self.max_marker.width() / 2.0,
                          self.max_marker.top(),
                          self.max_marker.width() / 2.0,
                          self.max_marker.height())

        else:
            return QRectF(self.max_marker.left(),
                          self.max_marker.top(),
                          self.max_marker.width(),
                          self.max_marker.height() / 2.0)

    def mouseMoveEvent (self, mouse_event):
        """Dragging or highlighting the markers."""
        self.setDefaultColors()

        min_test_marker = self._getMinTestMarker()

        if min_test_marker.contains(mouse_event.x(), mouse_event.y()) or self.selected_marker == 'min':
            self.min_marker_brush = self.getDefaultHighlightColor()

        if self.selected_marker == 'min':
            if self.horizontal:
                value = mouse_event.x() / float(self.width())
            else:
                value = (self.height() - mouse_event.y()) / float(self.height())

            self.setMinValue(value, False)

        max_test_marker = self._getMaxTestMarker()

        if max_test_marker.contains(mouse_event.x(), mouse_event.y()) or self.selected_marker == 'max':
            self.max_marker_brush = self.getDefaultHighlightColor()

        if self.selected_marker == 'max':
            if self.horizontal:
                value = mouse_event.x() / float(self.width())
            else:
                value = (self.height() - mouse_event.y()) / float(self.height())
                
            self.setMaxValue(value, False)

        self.update()


    def mousePressEvent (self, mouse_event):
        """Selecting a marker."""
        if mouse_event.button() == Qt.LeftButton:
            min_test_marker = self._getMinTestMarker()

            if min_test_marker.contains(mouse_event.x(), mouse_event.y()):
                self.selected_marker = 'min'

            max_test_marker = self._getMaxTestMarker()

            if max_test_marker.contains(mouse_event.x(), mouse_event.y()):
                self.selected_marker = 'max'


    def mouseReleaseEvent (self, mouse_event):
        self.selected_marker = 'none'

    def leaveEvent (self, event):
        self.setDefaultColors()

    def getDefaultMarkerColor(self):
        return QApplication.palette().background().color().light(175)

    def getDefaultHighlightColor(self):
        return QApplication.palette().highlight().color()

    def setDefaultColors(self):
        self.min_marker_brush = self.getDefaultMarkerColor()
        self.max_marker_brush = self.getDefaultMarkerColor()
        self.update()

    def setMaxValue(self, max_value, update=True):
        """The the position of the max marker."""
        if self.horizontal:
            m = float(self.width())
        else:
            m = float(self.height())

        marker_offset = (self.size + 1) / m

        if not self.max_value == max_value:
            self.max_value = max_value
            if self.max_value - marker_offset <= self.min_value:
                self.max_value = self.min_value + marker_offset
            if self.max_value > 1.0:
                self.max_value = 1


            self.emit(SIGNAL('zoomValueChanged(float, float)'), self.min_value, self.max_value)

            if update:
                self.update()

    def setMinValue(self, min_value, update=True):
        """The the position of the min marker."""
        if self.horizontal:
            m = float(self.width())
        else:
            m = float(self.height())

        marker_offset = (self.size + 1) / m

        if not self.min_value == min_value:
            self.min_value = min_value
            if self.min_value + marker_offset >= self.max_value:
                self.min_value = self.max_value - marker_offset
            if self.min_value < 0.0:
                self.min_value = 0.0


            self.emit(SIGNAL('zoomValueChanged(float, float)'), self.min_value, self.max_value)

            if update:
                self.update()

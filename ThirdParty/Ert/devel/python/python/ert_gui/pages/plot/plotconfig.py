#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'plotconfig.py' is part of ERT - Ensemble based Reservoir Tool. 
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


from PyQt4.QtCore import QObject, QSize, pyqtSignal
from PyQt4.Qt import SIGNAL
from PyQt4.QtGui import QFormLayout, QFrame, QComboBox, QHBoxLayout, QDoubleSpinBox, QWidget, QPainter, QColor, QColorDialog
from PyQt4.QtGui import QCheckBox, QDialog

class PlotConfig(object):
    """
    Plot config represents settings information relating to plot lines.
    All values are available as properties.
    """

    def __init__(self, name, linestyle="-", marker="", color=(0.0, 0.0, 0.0), alpha=0.75, zorder=1, picker=None, visible=True):
        self._name = name
        self._linestyle = linestyle
        self._marker = marker
        self._color = color
        self._alpha = alpha
        self._is_visible = visible
        self._z_order = zorder
        self._picker = picker

        self.signal_handler = QObject()

    def notify(self):
        """Tell all listeners that something has changed. Automatically called by setters."""
        #self.signal_handler.emit(plotConfigChanged)
        self.signal_handler.emit(SIGNAL('plotConfigChanged(PlotConfig)'), self)

    def get_name(self):
        return self._name

    name = property(get_name, doc="The name of this plot.")

    def hasStyle(self):
        return not self.style == ""

    def get_style(self):
        return (str(self._marker) + str(self._linestyle)).strip()

    style = property(get_style, doc="Returns the combined style of line style and marker style.")

    def setLinestyle(self, linestyle):
        self._linestyle = linestyle
        self.notify()

    def getLinestyle(self):
        return self._linestyle

    linestyle = property(getLinestyle, setLinestyle, doc="Line style")

    def setMarker(self, marker):
        self._marker = marker
        self.notify()

    def getMarker(self):
        return self._marker

    marker = property(getMarker, setMarker, doc="Marker style")

    def setAlpha(self, alpha):
        self._alpha = alpha
        self.notify()

    def getAlpha(self):
        return self._alpha

    alpha = property(getAlpha, setAlpha, doc="Transparency of the line. 1 = Opaque ... 0 transparent.")

    def setColor(self, color):
        self._color = color
        self.notify()

    def getColor(self):
        return self._color

    color = property(getColor, setColor, doc="Color of the line.")


    def set_is_visible(self, is_visible):
        self._is_visible = is_visible
        self.notify()

    def is_visible(self):
        return self._is_visible

    is_visible = property(is_visible, set_is_visible, doc="Hide or show the plotline.")

    def set_z_order(self, z_order):
        self._z_order = z_order
        self.notify()

    def get_z_order(self):
        return self._z_order

    z_order = property(get_z_order, set_z_order, doc="Z drawing order. 10 = top ... 1 = bottom.")

    def setPicker(self, picker):
        self._picker = picker
        self.notify()

    def getPicker(self):
        return self._picker

    picker = property(getPicker, setPicker, doc="Picker radius")


class PlotConfigPanel(QFrame):
    """A panel to interact with PlotConfig instances."""
    plot_marker_styles = ["", ".", ",", "o", "*", "s", "+", "x", "p", "h", "H", "D", "d"]
    plot_line_styles = ["", "-", "--", "-.", ":"]

    changed = pyqtSignal(object)

    def __init__(self, plot_config):
        QFrame.__init__(self)
        self.plot_config = plot_config
        self.changed.connect( self._fetchValues )
        layout = QFormLayout()
        layout.setRowWrapPolicy(QFormLayout.WrapLongRows)

        self.chk_visible = QCheckBox()
        layout.addRow("Visible:", self.chk_visible)
        self.connect(self.chk_visible, SIGNAL('stateChanged(int)'), self._setVisibleState)

        self.plot_linestyle = QComboBox()
        self.plot_linestyle.addItems(self.plot_line_styles)
        self.connect(self.plot_linestyle, SIGNAL("currentIndexChanged(QString)"), self._setLineStyle)
        layout.addRow("Line style:", self.plot_linestyle)

        self.plot_marker_style = QComboBox()
        self.plot_marker_style.addItems(self.plot_marker_styles)
        self.connect(self.plot_marker_style, SIGNAL("currentIndexChanged(QString)"), self._setMarker)
        layout.addRow("Marker style:", self.plot_marker_style)



        self.alpha_spinner = QDoubleSpinBox(self)
        self.alpha_spinner.setMinimum(0.0)
        self.alpha_spinner.setMaximum(1.0)
        self.alpha_spinner.setDecimals(3)
        self.alpha_spinner.setSingleStep(0.01)

        self.connect(self.alpha_spinner, SIGNAL('valueChanged(double)'), self._setAlpha)
        layout.addRow("Blend factor:", self.alpha_spinner)

        self.color_picker = ColorPicker(plot_config)
        layout.addRow("Color:", self.color_picker)

        self.setLayout(layout)
        self._fetchValues(plot_config)

    def _fetchValues(self, plot_config):
        """Fetch values from a PlotConfig and insert into the panel."""
        self.plot_config = plot_config

        #block signals to avoid updating the incoming plot_config 

        state = self.plot_linestyle.blockSignals(True)
        linestyle_index = self.plot_line_styles.index(self.plot_config.linestyle)
        self.plot_linestyle.setCurrentIndex(linestyle_index)
        self.plot_linestyle.blockSignals(state)

        state = self.plot_marker_style.blockSignals(True)
        marker_index = self.plot_marker_styles.index(self.plot_config.marker)
        self.plot_marker_style.setCurrentIndex(marker_index)
        self.plot_marker_style.blockSignals(state)

        state = self.alpha_spinner.blockSignals(True)
        self.alpha_spinner.setValue(self.plot_config.alpha)
        self.alpha_spinner.blockSignals(state)

        state = self.chk_visible.blockSignals(True)
        self.chk_visible.setChecked(self.plot_config.is_visible)
        self.chk_visible.blockSignals(state)

        self.color_picker.update()

    #-------------------------------------------
    # update plot config from widgets
    #-------------------------------------------
    def _setLineStyle(self, linestyle):
        self.plot_config.linestyle = linestyle

    def _setMarker(self, marker):
        self.plot_config.marker = marker

    def _setAlpha(self, alpha):
        self.plot_config.alpha = alpha

    def _setVisibleState(self, state):
        self.plot_config.is_visible = state == 2
   


class ColorPicker(QWidget):
    """A widget that shows a colored box and pops up a color dialog."""

    def __init__(self, plot_config):
        QWidget.__init__(self)

        self.plot_config = plot_config
        self.color_dialog = QColorDialog()
        size = 20
        self.setMaximumSize(QSize(size, size))
        self.setMinimumSize(QSize(size, size))
        self.setToolTip("Click to change color!")

    def paintEvent(self, paintevent):
        """Paints the box"""
        painter = QPainter(self)

        rect = self.contentsRect()
        rect.setWidth(rect.width() - 1)
        rect.setHeight(rect.height() - 1)
        painter.drawRect(rect)

        rect.setX(rect.x() + 1)
        rect.setY(rect.y() + 1)
        painter.fillRect(rect, self._getColor())

    def _setColor(self, color):
        """Set the color of this picker."""
        self.plot_config.color = (color.redF(), color.greenF(), color.blueF())
        self.update()

    def _getColor(self):
        """Return the color of this picker as QColor."""
        color = self.plot_config.color
        return QColor(int(color[0] * 255), int(color[1] * 255), int(color[2] * 255))

    def mouseDoubleClickEvent(self, event):
        self.color_dialog.setCurrentColor(self._getColor())
        status = self.color_dialog.exec_()
        if status == QDialog.Accepted:
            self._setColor(self.color_dialog.selectedColor())

    def mousePressEvent(self, event):
        self.color_dialog.setCurrentColor(self._getColor())
        status = self.color_dialog.exec_()
        if status == QDialog.Accepted:
            self._setColor(self.color_dialog.selectedColor())


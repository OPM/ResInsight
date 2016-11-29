from PyQt4.QtCore import QSize
from PyQt4.QtGui import QWidget, QPainter, QHBoxLayout, QLabel


class LegendMarker(QWidget):
    """A widget that shows a colored box"""
    def __init__(self, color):
        QWidget.__init__(self)

        self.setMaximumSize(QSize(12, 12))
        self.setMinimumSize(QSize(12, 12))

        self.color = color

    def paintEvent(self, paintevent):
        """Paints the box"""
        painter = QPainter(self)

        rect = self.contentsRect()
        rect.setWidth(rect.width() - 1)
        rect.setHeight(rect.height() - 1)
        painter.drawRect(rect)

        rect.setX(rect.x() + 1)
        rect.setY(rect.y() + 1)
        painter.fillRect(rect, self.color)

class Legend(QWidget):
    """Combines a LegendMarker with a label"""
    def __init__(self, legend, color):
        QWidget.__init__(self)

        self.setMinimumWidth(140)
        self.setMaximumHeight(25)

        self.legend = legend

        layout = QHBoxLayout()
        layout.setMargin(0)

        self.legend_marker = LegendMarker(color)
        self.legend_marker.setToolTip(legend)

        layout.addWidget(self.legend_marker)
        self.legend_label = QLabel(legend)
        layout.addWidget(self.legend_label)
        layout.addStretch()

        self.setLayout(layout)

    def setLegend(self, legend):
        self.legend_label.setText(legend)

    def updateLegend(self, *args):
        legend_text = self.legend % args
        self.legend_label.setText(legend_text)
        self.legend_marker.setToolTip(legend_text)

    def setColor(self, color):
        self.legend_marker.color = color
        self.legend_marker.update()
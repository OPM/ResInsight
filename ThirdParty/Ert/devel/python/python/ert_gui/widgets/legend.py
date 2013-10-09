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

        layout = QHBoxLayout()

        legendMarker = LegendMarker(color)
        legendMarker.setToolTip(legend)

        layout.addWidget(legendMarker)
        layout.addWidget(QLabel(legend))
        layout.addStretch()

        self.setLayout(layout)
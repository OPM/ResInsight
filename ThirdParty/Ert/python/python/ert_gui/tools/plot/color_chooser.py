from PyQt4.QtCore import QSize, QRect, pyqtSignal
from PyQt4.QtGui import QWidget, QPainter, QHBoxLayout, QLabel, QFrame, QColor, QColorDialog


class ColorBox(QFrame):
    colorChanged = pyqtSignal(QColor)

    """A widget that shows a colored box"""
    def __init__(self, color, size=15):
        QFrame.__init__(self)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setMaximumSize(QSize(size, size))
        self.setMinimumSize(QSize(size, size))

        self._tile_colors = [QColor(255, 255, 255), QColor(200, 200, 255)]
        self._color = color

    def paintEvent(self, paint_event):
        """Paints the box"""
        painter = QPainter(self)
        rect = self.contentsRect()
        tile_count = 3
        tile_size = rect.width() / tile_count
        painter.save()
        painter.translate(rect.x(), rect.y())

        for y in range(tile_count):
            for x in range(tile_count):
                color_index = (y * tile_count + x) % 2
                tile_rect = QRect(x * tile_size, y * tile_size, tile_size, tile_size)
                painter.fillRect(tile_rect, self._tile_colors[color_index])

        painter.restore()
        painter.fillRect(rect, self._color)

        QFrame.paintEvent(self, paint_event)

    def mouseReleaseEvent(self, QMouseEvent):
        color = QColorDialog.getColor(self._color, self, "Select color", QColorDialog.ShowAlphaChannel)

        if color.isValid():
            self._color = color
            self.update()
            self.colorChanged.emit(self._color)

    @property
    def color(self):
        """ @rtype: QColor """
        return self._color

    @color.setter
    def color(self, color):
        self._color = QColor(color)
        self.update()

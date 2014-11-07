from PyQt4.QtCore import QSize, QRect, pyqtSignal
from PyQt4.QtGui import QWidget, QPainter, QHBoxLayout, QLabel, QFrame, QColor, QColorDialog


class ColorBox(QFrame):
    colorChanged = pyqtSignal(QColor)

    """A widget that shows a colored box"""
    def __init__(self, color):
        QFrame.__init__(self)
        self.setFrameStyle(QFrame.Panel | QFrame.Sunken)
        self.setMaximumSize(QSize(15, 15))
        self.setMinimumSize(QSize(15, 15))

        self.tile_size = 7
        self.tile_colors = [QColor(255, 255, 255), QColor(200, 200, 255)]

        self.color = color

    def paintEvent(self, paint_event):
        """Paints the box"""
        painter = QPainter(self)
        rect = self.contentsRect()
        tile_count = 3
        tile_size = rect.width() / tile_count
        painter.save()
        painter.translate(rect.x(), rect.y())

        color_index = 0
        for y in range(tile_count):

            if y % 2 == 1:
                color_index = 1
            else:
                color_index = 0

            for x in range(tile_count):
                tile_rect = QRect(x * tile_size, y * tile_size, tile_size, tile_size)
                painter.fillRect(tile_rect, self.tile_colors[color_index])

                color_index += 1
                if color_index >= len(self.tile_colors):
                    color_index = 0

        painter.restore()
        painter.fillRect(rect, self.color)

        QFrame.paintEvent(self, paint_event)

    def mouseReleaseEvent(self, QMouseEvent):
        color = QColorDialog.getColor(self.color, self, "Select color", QColorDialog.ShowAlphaChannel)

        if color.isValid():
            self.color = color
            self.update()
            self.colorChanged.emit(self.color)


class ColorChooser(QWidget):
    colorChanged = pyqtSignal(QColor)

    """Combines a ColorBox with a label"""
    def __init__(self, label, color):
        QWidget.__init__(self)

        self.setMinimumWidth(140)
        self.setMaximumHeight(25)

        layout = QHBoxLayout()
        layout.setMargin(0)
        layout.setSpacing(2)

        self.color_box = ColorBox(color)
        self.color_box.setToolTip("Click to change color.")
        self.color_box.colorChanged.connect(self.colorChanged)

        layout.addWidget(self.color_box)
        self.color_label = QLabel(label)
        layout.addWidget(self.color_label)
        layout.addStretch()

        self.setLayout(layout)
        self.label = label

    def setColor(self, color):
        self.color_box.color = color
        self.color_box.update()
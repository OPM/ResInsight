from math import floor
from PyQt4.QtGui import QPainter, QColor, QFrame, QLinearGradient


class SimpleProgress(QFrame):
    def __init__(self):
        QFrame.__init__(self)
        self.setLineWidth(1)
        self.setFrameStyle(QFrame.Panel | QFrame.Plain)

        self.background_color = QColor(255, 255, 255)
        # self.color = QColor(0, 128, 255)
        self.color = QColor(255, 200, 128)

        self.setMinimumHeight(15)
        self.setMaximumHeight(15)

        self.__progress = 0

        self.__shiny = False

    def setProgress(self, progress):
        self.__progress = progress
        self.update()

    def paintEvent(self, paint_event):
        QFrame.paintEvent(self, paint_event)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.setRenderHint(QPainter.SmoothPixmapTransform, True)

        rect = self.contentsRect()
        """@type: QRect"""

        painter.fillRect(rect, self.background_color)

        x = rect.x()
        y = rect.y()
        height = rect.height()
        width = floor(rect.width() * self.__progress)

        painter.fillRect(x, y, width, height, self.color)

        if self.__shiny:
            #Shiny overlay!
            gradient = QLinearGradient(rect.width() / 2, 0, rect.width() / 2, rect.height())
            gradient.setColorAt(0, QColor(255, 255, 255, 0))
            gradient.setColorAt(0.2, QColor(255, 255, 255, 200))
            gradient.setColorAt(0.4, QColor(255, 255, 255, 0))
            gradient.setColorAt(0.85, QColor(255, 255, 255, 0))
            gradient.setColorAt(0.85, QColor(0, 0, 0, 0))
            gradient.setColorAt(1, QColor(0, 0, 0, 127))
            painter.fillRect(rect, gradient)



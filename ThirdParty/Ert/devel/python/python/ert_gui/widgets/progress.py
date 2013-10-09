from math import floor
from PyQt4.QtGui import QWidget, QPainter, QColor, QFrame, QLinearGradient


class StateTracker(object):
    def __init__(self, state_reference, color, progress=0.0):
        super(StateTracker, self).__init__()
        self.name = state_reference
        self.color = color
        self.progress = progress

    def setProgress(self, progress):
        self.progress = progress


class Progress(QFrame):
    def __init__(self):
        QFrame.__init__(self)
        self.setLineWidth(1)
        self.setFrameStyle(QFrame.Panel | QFrame.Plain)

        self.color = QColor(255, 255, 255)

        self.setMinimumHeight(30)
        self.__state_order = []
        """@type: list of State"""

        self.__states = {}
        """@type: dict of (object, State)"""

    def addState(self, state, state_color, progress=0.0):
        state_tracker = StateTracker(state, state_color, progress)
        self.__state_order.append(state_tracker)
        self.__states[state] = state_tracker

    def updateState(self, state, progress):
        self.__states[state].setProgress(progress)
        self.update()

    def paintEvent(self, paintevent):
        QFrame.paintEvent(self, paintevent)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing, True)
        painter.setRenderHint(QPainter.SmoothPixmapTransform, True)

        rect = self.contentsRect()
        """@type: QRect"""

        painter.fillRect(rect, self.color)

        x = rect.x()
        y = rect.y()
        height = rect.height()
        count = len(self.__state_order)
        for index in range(count):
            state = self.__state_order[index]
            width = floor(rect.width() * (state.progress / 100.0))

            if index == count - 1:
                width = rect.width() - x + 1

            painter.fillRect(x, y, width, height, state.color)

            x += width

        #Shiny overlay!
        gradient = QLinearGradient(rect.width() / 2, 0, rect.width() / 2, rect.height())
        gradient.setColorAt(0, QColor(255, 255, 255, 0))
        gradient.setColorAt(0.2, QColor(255, 255, 255, 200))
        gradient.setColorAt(0.4, QColor(255, 255, 255, 0))
        gradient.setColorAt(0.85, QColor(255, 255, 255, 0))
        gradient.setColorAt(0.85, QColor(0, 0, 0, 0))
        gradient.setColorAt(1, QColor(0, 0, 0, 127))
        painter.fillRect(rect, gradient)



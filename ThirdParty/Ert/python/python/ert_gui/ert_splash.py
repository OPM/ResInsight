from PyQt4.QtCore import Qt
from PyQt4.QtGui import QSplashScreen, QApplication, QColor, QPen, QFont

from ert_gui.ertwidgets import resourceImage


class ErtSplash(QSplashScreen):
    def __init__(self):
        QSplashScreen.__init__(self)
        self.setWindowFlags(Qt.WindowStaysOnTopHint | Qt.SplashScreen)

        splash_width = 720
        splash_height = 400

        desktop = QApplication.desktop()
        screen = desktop.screenGeometry(desktop.primaryScreen()).size()

        screen_width, screen_height = screen.width(), screen.height()
        x = screen_width / 2 - splash_width / 2
        y = screen_height / 2 - splash_height / 2
        self.setGeometry(x, y, splash_width, splash_height)


        self.splash_image = resourceImage("splash.jpg")

        self.ert = "ERT"
        self.ert_title = "Ensemble based Reservoir Tool"
        self.version = "Version string"
        self.timestamp = "Timestamp string"
        self.copyright = u"Copyright \u00A9 2017 Statoil ASA, Norway"


    def drawContents(self, painter):
        """ @type painter: QPainter """
        w = self.width()
        h = self.height()

        margin = 10

        background = QColor(210, 211, 215)
        text_color = QColor(0, 0, 0)
        foreground = QColor(255, 255, 255)

        painter.setBrush(background)
        painter.fillRect(0, 0, w, h, background)


        pen = QPen()
        pen.setWidth(2)
        pen.setColor(foreground)


        painter.setPen(pen)
        painter.drawRect(0, 0, w - 1, h - 1)

        image_width = self.splash_image.width()
        image_height = self.splash_image.height()
        aspect = float(image_width) / float(image_height)

        scaled_height = h - 2 * margin
        scaled_width = scaled_height * aspect

        painter.drawRect(margin, margin, scaled_width, scaled_height)
        painter.drawPixmap(margin, margin, scaled_width, scaled_height, self.splash_image)

        text_x = scaled_width + 2 * margin
        top_offset = margin
        text_area_width = w - scaled_width - 2 * margin

        painter.setPen(text_color)


        text_size = 150
        font = QFont("Serif")
        font.setStyleHint(QFont.Serif)
        font.setPixelSize(text_size)
        painter.setFont(font)
        painter.drawText(text_x, margin + top_offset, text_area_width, text_size, Qt.AlignHCenter | Qt.AlignCenter, self.ert)

        top_offset += text_size + 2 * margin
        text_size = 25
        font.setPixelSize(text_size)
        painter.setFont(font)
        painter.drawText(text_x, top_offset, text_area_width, text_size, Qt.AlignHCenter | Qt.AlignCenter, self.ert_title)

        top_offset += text_size + 4 * margin
        text_size = 20
        font.setPixelSize(text_size)
        painter.setFont(font)
        painter.drawText(text_x, top_offset, text_area_width, text_size, Qt.AlignHCenter | Qt.AlignCenter, self.version)

        top_offset += text_size + margin
        text_size = 15
        font.setPixelSize(text_size)
        painter.setFont(font)
        painter.drawText(text_x, top_offset, text_area_width, text_size, Qt.AlignHCenter | Qt.AlignCenter, self.timestamp)


        text_size = 12
        font.setPixelSize(text_size)
        painter.setFont(font)
        painter.drawText(text_x, h - text_size - margin - 5, text_area_width, text_size + 5, Qt.AlignHCenter | Qt.AlignCenter, self.copyright)





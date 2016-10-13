from PyQt4.QtCore import QString, QSize, Qt
from PyQt4.QtGui import QPushButton, QColor, QLineEdit, QStyle

from ert_gui.ertwidgets import resourceIcon


class ClearableLineEdit(QLineEdit):
    passive_color = QColor(194, 194, 194)

    def __init__(self, placeholder="yyyy-mm-dd"):
        QLineEdit.__init__(self)

        self._placeholder_text = placeholder
        self._active_color = self.palette().color(self.foregroundRole())
        self._placeholder_active = False

        self._clear_button = QPushButton(self)
        self._clear_button.setIcon(resourceIcon("remove_favorite.png"))
        self._clear_button.setFlat(True)
        self._clear_button.setFocusPolicy(Qt.NoFocus)
        self._clear_button.setFixedSize(17, 17)
        self._clear_button.setCursor(Qt.ArrowCursor)

        self._clear_button.clicked.connect(self.clearButtonClicked)
        self._clear_button.setVisible(False)

        self.textChanged.connect(self.toggleClearButtonVisibility)

        self.showPlaceholder()


    def toggleClearButtonVisibility(self):
        self._clear_button.setVisible(len(str(self.text())) > 0 and not self._placeholder_active)

    def sizeHint(self):
        size = QLineEdit.sizeHint(self)
        return QSize(size.width() + self._clear_button.width() + 3, size.height())

    def minimumSizeHint(self):
        size = QLineEdit.minimumSizeHint(self)
        return QSize(size.width() + self._clear_button.width() + 3, size.height())

    def resizeEvent(self, event):
        right = self.rect().right()
        frame_width = self.style().pixelMetric(QStyle.PM_DefaultFrameWidth)
        self._clear_button.move(right - frame_width - self._clear_button.width(), (self.height() - self._clear_button.height()) / 2)
        QLineEdit.resizeEvent(self, event)

    def clearButtonClicked(self):
        self.setText("")

    def showPlaceholder(self):
        if not self._placeholder_active:
            self._placeholder_active = True
            QLineEdit.setText(self, self._placeholder_text)
            palette = self.palette()
            palette.setColor(self.foregroundRole(), self.passive_color)
            self.setPalette(palette)

    def hidePlaceHolder(self):
        if self._placeholder_active:
            self._placeholder_active = False
            QLineEdit.setText(self, "")
            palette = self.palette()
            palette.setColor(self.foregroundRole(), self._active_color)
            self.setPalette(palette)

    def focusInEvent(self, focus_event):
        QLineEdit.focusInEvent(self, focus_event)
        self.hidePlaceHolder()

    def focusOutEvent(self, focus_event):
        QLineEdit.focusOutEvent(self, focus_event)
        if str(QLineEdit.text(self)) == "":
            self.showPlaceholder()

    def keyPressEvent(self, key_event):
        if key_event.key() == Qt.Key_Escape:
            self.clear()
            self.clearFocus()
            key_event.accept()

        QLineEdit.keyPressEvent(self, key_event)

    def setText(self, string):
        self.hidePlaceHolder()

        QLineEdit.setText(self, string)

        if len(str(string)) == 0 and not self.hasFocus():
            self.showPlaceholder()

    def text(self):
        if self._placeholder_active:
            return QString("")
        else:
            return QLineEdit.text(self)

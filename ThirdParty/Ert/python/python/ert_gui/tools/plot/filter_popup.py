from PyQt4.QtCore import Qt, pyqtSignal
from PyQt4.QtGui import QWidget, QFrame, QDialog, QVBoxLayout, QCheckBox, QLabel, QLayout, QCursor


class FilterPopup(QDialog):
    filterSettingsChanged = pyqtSignal(dict)

    def __init__(self, parent=None):
        QDialog.__init__(self, parent, Qt.WindowStaysOnTopHint | Qt.X11BypassWindowManagerHint | Qt.FramelessWindowHint)
        self.setVisible(False)

        self.filter_items = {}

        layout = QVBoxLayout()
        layout.setMargin(0)
        frame = QFrame()
        frame.setFrameStyle(QFrame.StyledPanel | QFrame.Raised)
        layout.addWidget(frame)

        self.__layout = QVBoxLayout()
        self.__layout.setSizeConstraint(QLayout.SetFixedSize)
        self.__layout.addWidget(QLabel("Filter by data type:"))

        self.addFilterItem("Summary", "summary")
        # self.addFilterItem("Block", "block")
        self.addFilterItem("Gen KW", "gen_kw")
        self.addFilterItem("Gen Data", "gen_data")
        self.addFilterItem("Custom KW", "custom_kw")
        # self.addFilterItem("Custom PCA", "custom_pca")

        frame.setLayout(self.__layout)

        self.setLayout(layout)
        self.adjustSize()


    def addFilterItem(self, name, id, value=True):
        self.filter_items[id] = value

        check_box = QCheckBox(name)
        check_box.setChecked(value)

        def toggleItem(checked):
            self.filter_items[id] = checked
            self.filterSettingsChanged.emit(self.filter_items)

        check_box.toggled.connect(toggleItem)

        self.__layout.addWidget(check_box)

    def leaveEvent(self, QEvent):
        QWidget.leaveEvent(self, QEvent)
        self.hide()

    def show(self):
        QWidget.show(self)
        p = QCursor().pos()
        self.move(p.x(), p.y())

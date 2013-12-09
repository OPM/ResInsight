from PyQt4.QtCore import Qt, SIGNAL, QSize
from PyQt4.QtGui import QWidget, QGridLayout, QGroupBox, QLabel, QVBoxLayout, QFrame, QHBoxLayout, QTabWidget
from ert_gui.pages.popup_dialog import PopupDialog


class RowPanel(QWidget):
    def __init__(self, name):
        QWidget.__init__(self)
        self.__name = name
        self.__layout_queue = []
        self.setMinimumWidth(500)
        self.__layout = QGridLayout()
        """ @type: QGridLayout """
        self.setLayout(self.__layout)
        self.__row = 0
        self.__widgets = {}

        self.__layout.setColumnMinimumWidth(0, 20)
        self.__layout.setColumnMinimumWidth(6, 20)

        self.__popups = []

    def getName(self):
        return self.__name

    def __startNewRow(self):
        self.__layout.setRowStretch(self.__row, 0)

    def __rowFinished(self):
        self.__row += 2
        self.__layout.setRowStretch(self.__row, 1)

    def addRow(self, row_widget, configurator=None, configuration_title="Configure"):
        """
        Add a new row on a configuration page. Returns the row widget.
        If the row does not have a getLabel() function the row spans both columns.

        @rtype: QWidget
        """
        assert hasattr(row_widget, "getLabel"), "Widget must have a getLabel() method"

        self.__startNewRow()

        if row_widget.getLabel() is None or row_widget.getLabel() == "":
            self.__layout.addWidget(row_widget, self.__row, 1, 1, 3)
        else:
            self.__layout.addWidget(QLabel(row_widget.getLabel()), self.__row, 1, Qt.AlignLeft | Qt.AlignTop)
            self.__layout.addWidget(row_widget, self.__row, 3)

        if not configurator is None:
            self.__layout.setColumnMinimumWidth(4, 20)
            button = self.__addConfigurationButton(configurator, configuration_title)
            self.__layout.addWidget(button, self.__row, 5)

        self.__widgets[row_widget] = self.__row

        self.__rowFinished()


    def startTabs(self, name_of_first_tab):
        self.__tab_widget = QTabWidget()
        self.__layout_queue.append(self.__layout)
        self.__layout = None
        self.addTab(name_of_first_tab)

    def addTab(self, name):
        self.__layout = QGridLayout()
        widget = QWidget()
        widget.setLayout(self.__layout)
        self.__tab_widget.addTab(widget, name)


    def endTabs(self):
        self.__layout = self.__layout_queue.pop()
        """ @type: QGridLayout """
        self.__startNewRow()
        self.__layout.addWidget(self.__tab_widget, self.__row, 1, 1, 5)
        self.__rowFinished()
        self.__tab_widget = None


    def startGroup(self, group_title):
        """Start a titled sub group on the page."""
        self.__group_box = QGroupBox(group_title)
        self.__layout_queue.append(self.__layout)
        self.__layout = QGridLayout()

    def endGroup(self):
        """Finish the titled sub group"""
        self.__group_box.setLayout(self.__layout)
        self.__layout = self.__layout_queue.pop()
        """ @type: QGridLayout """
        self.__layout.addRow(self.__group_box)
        self.__group_box = None

    def addLabeledSeparator(self, label):
        """Adds a labeled separator line to the panel."""
        widget = QWidget()
        layout = QVBoxLayout()
        widget.setLayout(layout)

        h_layout = QHBoxLayout()

        frame = QFrame()
        frame.setFrameShape(QFrame.HLine)
        frame.setFrameShadow(QFrame.Sunken)

        h_layout.addWidget(QLabel(label))
        h_layout.addWidget(frame, 1, Qt.AlignBottom)

        layout.addSpacing(2)
        layout.addLayout(h_layout)
        layout.addSpacing(0)

        # widget.setStyleSheet("background-color: #ffff00")

        self.__startNewRow()
        self.__layout.addWidget(widget, self.__row, 0, 1, 7)
        self.__rowFinished()

    def addSeparator(self):
        widget = QWidget()
        layout = QVBoxLayout()
        widget.setLayout(layout)

        frame = QFrame()
        frame.setFrameShape(QFrame.HLine)
        frame.setFrameShadow(QFrame.Sunken)

        layout.addSpacing(5)
        layout.addWidget(frame)
        layout.addSpacing(5)

        self.__startNewRow()
        self.__layout.addWidget(widget, self.__row, 1, 1, 5)
        self.__rowFinished()


    def __addConfigurationButton(self, configurator, configuration_title):
        popup = PopupDialog(configurator.getName(), configurator, self)
        button = popup.getButton()
        button.setText(configuration_title)
        self.__popups.append(popup)
        return button


    def addSpace(self, size=5):
        """Creates a widget that can be used as spacing on  a panel."""
        space = QWidget()
        space.setMinimumSize(QSize(size, size))
        self.__startNewRow()
        self.__layout.addWidget(space, self.__row, 1, 1, 5)
        self.__rowFinished()



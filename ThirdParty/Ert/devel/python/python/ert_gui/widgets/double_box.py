class DoubleBox(HelpedWidget):
    """DoubleBox shows a double value. The data structure expected and sent to the getter and setter is a double."""

    def __init__(self, parent=None, pathLabel="Double", help=""):
        """Construct a DoubleBox widget"""
        HelpedWidget.__init__(self, parent, pathLabel, help)

        self.doubleBox = QtGui.QLineEdit()
        self.doubleBox.setValidator(QtGui.QDoubleValidator(self))
        self.doubleBox.setMaximumWidth(75)

        #self.connect(self.doubleBox, QtCore.SIGNAL('editingFinished()'), self.validateString)
        self.connect(self.doubleBox, QtCore.SIGNAL('editingFinished()'), self.contentsChanged)
        self.connect(self.doubleBox, QtCore.SIGNAL('textChanged(QString)'), self.validateString)
        self.addWidget(self.doubleBox)

        self.addStretch()
        self.addHelpButton()


    def validateString(self):
        stringToValidate = str(self.doubleBox.text())
        if stringToValidate.strip() == "":
            self.contentsChanged()

    def contentsChanged(self):
        """Called whenever the contents of the editline changes."""
        self.updateContent(str(self.doubleBox.text()))

    def fetchContent(self):
        """Retrieves data from the model and inserts it into the edit line"""
        self.doubleBox.setText(str(self.getFromModel()))
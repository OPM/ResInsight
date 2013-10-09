from PyQt4.QtCore import SIGNAL
from PyQt4.QtGui import QSpinBox, QLabel
from ert_gui.models.mixins import SpinnerModelMixin
from ert_gui.widgets.helped_widget import HelpedWidget


class IntegerSpinner(HelpedWidget):
    """A spinner widget for integers. The data structure expected and sent to the getter and setter is an integer."""
    def __init__(self, model, spinner_label="Integer Number", help_link=""):
        HelpedWidget.__init__(self, spinner_label, help_link)

        assert isinstance(model, SpinnerModelMixin)
        self.model = model
        model.observable().attach(SpinnerModelMixin.SPINNER_VALUE_CHANGED_EVENT, self.getValueFromModel)
        model.observable().attach(SpinnerModelMixin.RANGE_VALUE_CHANGED_EVENT, self.getRangeFromModel)

        self.spinner = QSpinBox(self)
        self.addWidget(self.spinner)

        self.info_label = QLabel()
        self.info_label.setHidden(True)
        self.addWidget(self.info_label)

        self.addStretch()

        # self.connect(self.spinner, SIGNAL('editingFinished()'), self.updateModel)
        self.__initialized = False
        self.connect(self.spinner, SIGNAL('valueChanged(int)'), self.updateModel)

        self.getRangeFromModel()
        self.getValueFromModel()

    def updateModel(self):
        """Called whenever the contents of the spinner changes."""
        if self.__initialized:
            self.model.setSpinnerValue(self.spinner.value())

    def getRangeFromModel(self):
        self.spinner.setMinimum(self.model.getMinValue())
        self.spinner.setMaximum(self.model.getMaxValue())

    def getValueFromModel(self):
        """Retrieves data from the model and inserts it into the spinner"""
        if not self.__initialized:
            self.__initialized = True
        self.spinner.setValue(self.model.getSpinnerValue())

    def setInfo(self, info):
        self.info_label.setText(info)
        self.info_label.setHidden(False)
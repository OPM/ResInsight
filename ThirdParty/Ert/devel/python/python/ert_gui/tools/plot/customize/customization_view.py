from PyQt4.QtGui import QWidget, QFormLayout, QSpacerItem, QCheckBox

from ert_gui.tools.plot import StyleChooser
from ert_gui.plottery import PlotConfig
from ert_gui.widgets.clearable_line_edit import ClearableLineEdit


class CustomizationView(QWidget):
    def __init__(self):
        QWidget.__init__(self)

        self._layout = QFormLayout()
        self.setLayout(self._layout)
        self._widgets = {}


    def addRow(self, title, widget):
        self._layout.addRow(title, widget)

    def addLineEdit(self, attribute_name, title, tool_tip=None, placeholder=""):
        self[attribute_name] = ClearableLineEdit(placeholder=placeholder)
        self.addRow(title, self[attribute_name])

        if tool_tip is not None:
            self[attribute_name].setToolTip(tool_tip)

        def getter(self):
            value = str(self[attribute_name].text())
            if value == "":
                value = None
            return value

        def setter(self, value):
            if value is None:
                value = ""
            self[attribute_name].setText(str(value))

        self.updateProperty(attribute_name, getter, setter)

    def addCheckBox(self, attribute_name, title, tool_tip=None):
        self[attribute_name] = QCheckBox()
        self.addRow(title, self[attribute_name])

        if tool_tip is not None:
            self[attribute_name].setToolTip(tool_tip)

        def getter(self):
            return self[attribute_name].isChecked()

        def setter(self, value):
            self[attribute_name].setChecked(value)

        self.updateProperty(attribute_name, getter, setter)


    def addStyleChooser(self, attribute_name, title, tool_tip=None, area_supported=False):
        style_chooser = StyleChooser(area_supported=area_supported)
        self[attribute_name] = style_chooser
        self.addRow(title, self[attribute_name])

        if tool_tip is not None:
            self[attribute_name].setToolTip(tool_tip)

        def getter(self):
            return self[attribute_name].getStyle()

        def setter(self, style):
            self[attribute_name].setStyle(style)

        self.updateProperty(attribute_name, getter, setter)



    def updateProperty(self, attribute_name, getter, setter):
        setattr(self.__class__, attribute_name, property(getter, setter))

    def setWidgetEnabled(self, attribute_name, enabled):
        widget = self[attribute_name]
        widget.setEnabled(enabled)
        widget.setHidden(enabled)
        label = self._layout.labelForField(widget)
        label.setEnabled(enabled)
        label.setHidden(enabled)

    def addSpacing(self, pixels=10):
        self._layout.addItem(QSpacerItem(1, pixels))

    def addHeading(self, title):
        self.addSpacing(10)
        self._layout.addRow(title, None)
        self.addSpacing(1)

    def __getitem__(self, item):
        """
        @rtype: QWidget
        """
        return self._widgets[item]

    def __setitem__(self, key, value):
        self._widgets[key] = value

    def applyCustomization(self, plot_config):
        """
        @type plot_config: PlotConfig
        """
        raise NotImplementedError("Class '%s' has not implemented the applyCustomization() function!" % self.__class__.__name__)

    def revertCustomization(self, plot_config):
        """
        @type plot_config: PlotConfig
        """
        raise NotImplementedError("Class '%s' has not implemented the revertCustomization() function!" % self.__class__.__name__)



class WidgetProperty(object):
    def __get__(self, instance, owner):
        raise UserWarning("Property is invalid!")

    def __set__(self, instance, value):
        raise UserWarning("Property is invalid!")

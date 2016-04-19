from PyQt4.QtGui import QDoubleValidator, QStackedWidget, QIntValidator, QLabel

from ert_gui.plottery import PlotContext
from ert_gui.plottery.plot_limits import PlotLimits
from ert_gui.tools.plot.customize import CustomizationView
from ert_gui.widgets.clearable_line_edit import ClearableLineEdit
from ert_gui.widgets.custom_date_edit import CustomDateEdit


class StackedInput(QStackedWidget):
    def __init__(self):
        QStackedWidget.__init__(self)
        self._inputs = {}
        self._index_map = {}
        self.addInput(PlotContext.UNKNOWN_AXIS, QLabel("Fixed"))
        self._current_name = PlotContext.UNKNOWN_AXIS

    def addInput(self, name, widget):
        index = self.addWidget(widget)
        self._inputs[name] = widget
        self._index_map[name] = index

    def switchToInput(self, name):
        index_for_name = self._index_map[name]
        self.setCurrentIndex(index_for_name)
        self._current_name = name

    def getCurrentInputName(self):
        return self._current_name


class LimitsStack(StackedInput):
    FLOAT_AXIS = [PlotContext.VALUE_AXIS, PlotContext.DENSITY_AXIS, PlotContext.DEPTH_AXIS]
    INT_AXIS = [PlotContext.INDEX_AXIS, PlotContext.COUNT_AXIS]
    NUMBER_AXIS = FLOAT_AXIS + INT_AXIS

    def __init__(self):
        StackedInput.__init__(self)
        self.addInput(PlotContext.COUNT_AXIS, self.createIntegerLineEdit(minimum=0, placeholder="Default value"))
        self.addInput(PlotContext.DATE_AXIS, CustomDateEdit())
        self.addInput(PlotContext.DENSITY_AXIS, self.createDoubleLineEdit(minimum=0, placeholder="Default value"))
        self.addInput(PlotContext.DEPTH_AXIS, self.createDoubleLineEdit(minimum=0, placeholder="Default value"))
        self.addInput(PlotContext.INDEX_AXIS, self.createIntegerLineEdit(minimum=0, placeholder="Default value"))
        self.addInput(PlotContext.VALUE_AXIS, self.createDoubleLineEdit(placeholder="Default value"))

    def createDoubleLineEdit(self, minimum=None, maximum=None, placeholder=""):
        line_edit = ClearableLineEdit(placeholder=placeholder)
        validator = QDoubleValidator()

        if minimum is not None:
            validator.setBottom(minimum)

        if maximum is not None:
            validator.setTop(maximum)

        line_edit.setValidator(validator)
        return line_edit

    def createIntegerLineEdit(self, minimum=None, maximum=None, placeholder=""):
        line_edit = ClearableLineEdit(placeholder=placeholder)
        validator = QIntValidator()

        if minimum is not None:
            validator.setBottom(minimum)

        if maximum is not None:
            validator.setTop(maximum)

        line_edit.setValidator(validator)
        return line_edit

    def setValue(self, axis_name, value):
        input = self._inputs[axis_name]

        if axis_name in LimitsStack.NUMBER_AXIS:
            if value is None:
                input.setText("")
            else:
                input.setText(str(value))
        elif axis_name == PlotContext.DATE_AXIS:
            input.setDate(value)

    def getValue(self, axis_name):
        input = self._inputs[axis_name]
        result = None
        if axis_name in LimitsStack.FLOAT_AXIS:
            try:
                result = float(input.text())
            except ValueError:
                result = None
        elif axis_name in LimitsStack.INT_AXIS:
            try:
                result = int(input.text())
            except ValueError:
                result = None
        elif axis_name == PlotContext.DATE_AXIS:
            result = input.date()

        return result


class LimitsWidget(object):
    def __init__(self):
        self._limits = PlotLimits()
        self._x_minimum_stack = LimitsStack()
        self._x_maximum_stack = LimitsStack()
        self._x_current_input_name = PlotContext.UNKNOWN_AXIS

        self._y_minimum_stack = LimitsStack()
        self._y_maximum_stack = LimitsStack()
        self._y_current_input_name = PlotContext.UNKNOWN_AXIS


    @property
    def x_minimum_stack(self):
        """ @rtype: LimitsStack """
        return self._x_minimum_stack

    @property
    def x_maximum_stack(self):
        """ @rtype: LimitsStack """
        return self._x_maximum_stack

    @property
    def y_minimum_stack(self):
        """ @rtype: LimitsStack """
        return self._y_minimum_stack

    @property
    def y_maximum_stack(self):
        """ @rtype: LimitsStack """
        return self._y_maximum_stack

    @property
    def limits(self):
        """ @rtype: PlotLimits """
        self._updateLimits()
        limits = PlotLimits()
        limits.copyLimitsFrom(self._limits)
        return limits

    @limits.setter
    def limits(self, value):
        """ @type value: PlotLimits """
        self._limits.copyLimitsFrom(value)
        self._updateWidgets()

    def _updateWidgets(self):
        limits = self._limits
        self._x_minimum_stack.setValue(PlotContext.DATE_AXIS, limits.date_minimum)
        self._x_maximum_stack.setValue(PlotContext.DATE_AXIS, limits.date_maximum)
        self._y_minimum_stack.setValue(PlotContext.DATE_AXIS, limits.date_minimum)
        self._y_maximum_stack.setValue(PlotContext.DATE_AXIS, limits.date_maximum)

        self._x_minimum_stack.setValue(PlotContext.DEPTH_AXIS, limits.depth_minimum)
        self._x_maximum_stack.setValue(PlotContext.DEPTH_AXIS, limits.depth_maximum)
        self._y_minimum_stack.setValue(PlotContext.DEPTH_AXIS, limits.depth_minimum)
        self._y_maximum_stack.setValue(PlotContext.DEPTH_AXIS, limits.depth_maximum)

        self._x_minimum_stack.setValue(PlotContext.DENSITY_AXIS, limits.density_minimum)
        self._x_maximum_stack.setValue(PlotContext.DENSITY_AXIS, limits.density_maximum)
        self._y_minimum_stack.setValue(PlotContext.DENSITY_AXIS, limits.density_minimum)
        self._y_maximum_stack.setValue(PlotContext.DENSITY_AXIS, limits.density_maximum)

        self._x_minimum_stack.setValue(PlotContext.COUNT_AXIS, limits.count_minimum)
        self._x_maximum_stack.setValue(PlotContext.COUNT_AXIS, limits.count_maximum)
        self._y_minimum_stack.setValue(PlotContext.COUNT_AXIS, limits.count_minimum)
        self._y_maximum_stack.setValue(PlotContext.COUNT_AXIS, limits.count_maximum)

        self._x_minimum_stack.setValue(PlotContext.INDEX_AXIS, limits.index_minimum)
        self._x_maximum_stack.setValue(PlotContext.INDEX_AXIS, limits.index_maximum)
        self._y_minimum_stack.setValue(PlotContext.INDEX_AXIS, limits.index_minimum)
        self._y_maximum_stack.setValue(PlotContext.INDEX_AXIS, limits.index_maximum)

        self._x_minimum_stack.setValue(PlotContext.VALUE_AXIS, limits.value_minimum)
        self._x_maximum_stack.setValue(PlotContext.VALUE_AXIS, limits.value_maximum)
        self._y_minimum_stack.setValue(PlotContext.VALUE_AXIS, limits.value_minimum)
        self._y_maximum_stack.setValue(PlotContext.VALUE_AXIS, limits.value_maximum)


    def _updateLimits(self):
        if self._x_current_input_name is not PlotContext.UNKNOWN_AXIS:
            minimum = self._x_minimum_stack.getValue(self._x_current_input_name)
            maximum = self._x_maximum_stack.getValue(self._x_current_input_name)
            self._updateLimit(self._x_current_input_name, minimum, maximum)

        if self._y_current_input_name is not PlotContext.UNKNOWN_AXIS:
            minimum = self._y_minimum_stack.getValue(self._y_current_input_name)
            maximum = self._y_maximum_stack.getValue(self._y_current_input_name)
            self._updateLimit(self._y_current_input_name, minimum, maximum)


    def _updateLimit(self, axis_name, minimum, maximum):
        if axis_name == PlotContext.COUNT_AXIS:
            self._limits.count_limits = minimum, maximum
        elif axis_name == PlotContext.DENSITY_AXIS:
            self._limits.density_limits = minimum, maximum
        elif axis_name == PlotContext.DEPTH_AXIS:
            self._limits.depth_limits = minimum, maximum
        elif axis_name == PlotContext.DATE_AXIS:
            self._limits.date_limits = minimum, maximum
        elif axis_name == PlotContext.INDEX_AXIS:
            self._limits.index_limits = minimum, maximum
        elif axis_name == PlotContext.VALUE_AXIS:
            self._limits.value_limits = minimum, maximum


    def switchInputOnX(self, axis_type):
        self._x_current_input_name = axis_type
        self._updateWidgets()
        self._x_minimum_stack.switchToInput(axis_type)
        self._x_maximum_stack.switchToInput(axis_type)


    def switchInputOnY(self, axis_type):
        self._y_current_input_name = axis_type
        self._updateWidgets()
        self._y_minimum_stack.switchToInput(axis_type)
        self._y_maximum_stack.switchToInput(axis_type)



class LimitsCustomizationView(CustomizationView):

    def __init__(self):
        CustomizationView.__init__(self)

        limits_widget = LimitsWidget()
        self._limits_widget = limits_widget

        self.addHeading("X Axis")
        self.addRow("Minimum", limits_widget.x_minimum_stack)
        self.addRow("Maximum", limits_widget.x_maximum_stack)

        self.addHeading("Y Axis")
        self.addRow("Minimum", limits_widget.y_minimum_stack)
        self.addRow("Maximum", limits_widget.y_maximum_stack)


    def setAxisTypes(self, x_axis_type, y_axis_type):
        self._limits_widget.switchInputOnX(x_axis_type)
        self._limits_widget.switchInputOnY(y_axis_type)

    def revertCustomization(self, plot_config):
        """ @type plot_config: ert_gui.plottery.PlotConfig """
        self._limits_widget.limits = plot_config.limits


    def applyCustomization(self, plot_config):
        """ @type plot_config: ert_gui.plottery.PlotConfig """
        plot_config.limits = self._limits_widget.limits

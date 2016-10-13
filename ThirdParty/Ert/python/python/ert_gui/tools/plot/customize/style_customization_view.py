from PyQt4.QtGui import QHBoxLayout, QLabel, QColor

from ert_gui.tools.plot import ColorBox
from ert_gui.tools.plot.customize import CustomizationView, WidgetProperty


class StyleCustomizationView(CustomizationView):
    default_style = WidgetProperty()
    refcase_style = WidgetProperty()
    history_style = WidgetProperty()
    color_cycle = WidgetProperty()

    def __init__(self):
        CustomizationView.__init__(self)

        layout = QHBoxLayout()

        self.addRow("", layout)
        self.addStyleChooser("default_style", "Default", "Line and marker style for default lines.")
        self.addStyleChooser("refcase_style", "Refcase", "Line and marker style for the refcase line.")
        self.addStyleChooser("history_style", "History", "Line and marker style for the history line.")

        self["default_style"].createLabelLayout(layout)

        self.addSpacing(10)

        color_layout = QHBoxLayout()

        self._color_boxes = []
        for name in ["#1", "#2", "#3", "#4", "#5"]:
            color_box = self.createColorBox(name)
            self._color_boxes.append(color_box)
            color_layout.addWidget(color_box)

        self.addRow("Color Cycle", color_layout)
        self.updateProperty("color_cycle", StyleCustomizationView.getColorCycle, StyleCustomizationView.setColorCycle)


    def createColorBox(self, name):
        color_box = ColorBox(QColor(255, 255, 255), 20)
        color_box.setToolTip(name)
        return color_box


    def getColorCycle(self):
        colors = []
        for color_box in self._color_boxes:
            colors.append(str(color_box.color.name()))

        return colors

    def setColorCycle(self, color_cycle):
        for index, color in enumerate(color_cycle):
            if 0 <= index < len(self._color_boxes):
                color_box = self._color_boxes[index]
                color_box.color = color

    def applyCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        plot_config.setDefaultStyle(self.default_style)
        plot_config.setRefcaseStyle(self.refcase_style)
        plot_config.setHistoryStyle(self.history_style)
        plot_config.setLineColorCycle(self.color_cycle)

    def revertCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        self.default_style = plot_config.defaultStyle()
        self.refcase_style = plot_config.refcaseStyle()
        self.history_style = plot_config.historyStyle()
        self.color_cycle = plot_config.lineColorCycle()



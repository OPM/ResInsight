from PyQt4.QtGui import QHBoxLayout, QLabel, QComboBox

from ert_gui.tools.plot.customize import CustomizationView, WidgetProperty


class StatisticsCustomizationView(CustomizationView):
    mean_style = WidgetProperty()
    p50_style = WidgetProperty()
    std_style = WidgetProperty()
    min_max_style = WidgetProperty()
    p10_p90_style = WidgetProperty()
    p33_p67_style = WidgetProperty()
    std_dev_factor = WidgetProperty()
    distribution_lines = WidgetProperty()


    def __init__(self):
        CustomizationView.__init__(self)

        self._presets = ["Statistics Default", "Cross Case Statistics Default", "Overview", "All statistics"]

        self.addRow("Presets", self.createPresets())
        self.addSpacing(10)
        layout = QHBoxLayout()
        self.addRow("", layout)
        self.addStyleChooser("mean_style", "Mean", "Line and marker style for the mean line.")
        self.addStyleChooser("p50_style", "P50", "Line and marker style for the P50 line.")
        self.addStyleChooser("std_style", "Std dev", "Line and marker style for the unbiased standard deviation lines.", True)
        self.addStyleChooser("min_max_style", "Min/Max", "Line and marker style for the min/max lines.", True)
        self.addStyleChooser("p10_p90_style", "P10-P90", "Line and marker style for the P10-P90 lines.", True)
        self.addStyleChooser("p33_p67_style", "P33-P67", "Line and marker style for the P33-P67 lines.", True)
        self.addSpacing()

        std_box = self.addSpinBox("std_dev_factor", "Std dev multiplier", "Choose which standard deviation to plot", max_value=3)

        self.addCheckBox("distribution_lines", "Connection Lines", "Toggle distribution connection lines visibility.")

        self["mean_style"].createLabelLayout(layout)



    def createPresets(self):
        preset_combo = QComboBox()
        for preset in self._presets:
            preset_combo.addItem(preset)

        preset_combo.currentIndexChanged.connect(self.presetSelected)
        return preset_combo


    def presetSelected(self, index):
        if index == 0: # Default
            self.updateStyle("mean_style", "-", None)
            self.updateStyle("p50_style", None, None)
            self.updateStyle("std_style", None, None)
            self.updateStyle("min_max_style", None, None)
            self.updateStyle("p10_p90_style", "--", None)
            self.updateStyle("p33_p67_style", None, None)
        elif index == 1: # CCS Default
            self.updateStyle("mean_style", "-", "o")
            self.updateStyle("p50_style", None, None)
            self.updateStyle("std_style", "--", "D")
            self.updateStyle("min_max_style", None, None)
            self.updateStyle("p10_p90_style", None, None)
            self.updateStyle("p33_p67_style", None, None)
        elif index == 2: # Overview
            self.updateStyle("mean_style", None, None)
            self.updateStyle("p50_style", None, None)
            self.updateStyle("std_style", None, None)
            self.updateStyle("min_max_style", "#", None)
            self.updateStyle("p10_p90_style", None, None)
            self.updateStyle("p33_p67_style", None, None)
        elif index == 3: # All statistics
            self.updateStyle("mean_style", "-", None)
            self.updateStyle("p50_style", "--", "x")
            self.updateStyle("std_style", ":", None)
            self.updateStyle("min_max_style", "--", None)
            self.updateStyle("p10_p90_style", "#", None)
            self.updateStyle("p33_p67_style", "#", None)


    def updateStyle(self, attribute_name, line_style, marker_style):
        style = getattr(self, attribute_name)
        style.line_style = line_style
        style.marker = marker_style
        setattr(self, attribute_name, style)


    def applyCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        plot_config.setStatisticsStyle("mean", self.mean_style)
        plot_config.setStatisticsStyle("p50", self.p50_style)
        plot_config.setStatisticsStyle("std", self.std_style)
        plot_config.setStatisticsStyle("min-max", self.min_max_style)
        plot_config.setStatisticsStyle("p10-p90", self.p10_p90_style)
        plot_config.setStatisticsStyle("p33-p67", self.p33_p67_style)

        plot_config.setStandardDeviationFactor(self.std_dev_factor)
        plot_config.setDistributionLineEnabled(self.distribution_lines)


    def revertCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        self.mean_style = plot_config.getStatisticsStyle("mean")
        self.p50_style = plot_config.getStatisticsStyle("p50")
        self.std_style = plot_config.getStatisticsStyle("std")
        self.min_max_style = plot_config.getStatisticsStyle("min-max")
        self.p10_p90_style = plot_config.getStatisticsStyle("p10-p90")
        self.p33_p67_style = plot_config.getStatisticsStyle("p33-p67")

        self.std_dev_factor = plot_config.getStandardDeviationFactor()
        self.distribution_lines = plot_config.isDistributionLineEnabled()

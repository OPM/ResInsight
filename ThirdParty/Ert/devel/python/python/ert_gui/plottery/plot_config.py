import os
import itertools
from ert_gui.plottery import PlotStyle, PlotLimits


class PlotConfig(object):

    def __init__(self, title="Unnamed", x_label=None, y_label=None):
        super(PlotConfig, self).__init__()
        self._title = title

        self._line_color_cycle_colors = ["#000000"]
        self._line_color_cycle = itertools.cycle(self._line_color_cycle_colors) #Black
        # Blueish, Greenlike, Beigeoid, Pinkness, Orangy-Brown
        self.setLineColorCycle(["#386CB0", "#7FC97F", "#FDC086", "#F0027F", "#BF5B17"])

        self._legend_items = []
        self._legend_labels = []

        self._x_label = x_label
        self._y_label = y_label

        self._limits = PlotLimits()

        self._default_style = PlotStyle(name="Default", color=None, alpha=0.8)
        self._refcase_style = PlotStyle(name="Refcase", alpha=0.8, marker="x", width=2.0)
        self._history_style = PlotStyle(name="History", alpha=0.8, marker="D", width=2.0)

        # Insanely ugly implementation of user preferences.
        if os.getenv("ERT_SHOW_HISTORY_VECTORS"):
            self._history_style.setEnabled(True)
        else:
            self._history_style.setEnabled(False)

        self._observation_style = PlotStyle(name="Observations")
        self._histogram_style = PlotStyle(name="Histogram", width=2.0)
        self._distribution_style = PlotStyle(name="Distribution", line_style="", marker="o", alpha=0.5, size=10.0)
        self._distribution_line_style = PlotStyle(name="Distribution Lines", line_style="-", alpha=0.25, width=1.0)
        self._distribution_line_style.setEnabled(False)
        self._current_color = None

        self._legend_enabled = True
        self._grid_enabled = True


        self._statistics_style = {
            "mean": PlotStyle("Mean", line_style=""),
            "p50": PlotStyle("P50", line_style=""),
            "min-max": PlotStyle("Min/Max", line_style=""),
            "p10-p90": PlotStyle("P10-P90", line_style=""),
            "p33-p67": PlotStyle("P33-P67", line_style=""),
            "std": PlotStyle("Std dev", line_style="")
        }

    def currentColor(self):
        if self._current_color is None:
            self.nextColor()

        return self._current_color

    def nextColor(self):
        self._current_color = self._line_color_cycle.next()
        return self._current_color

    def setLineColorCycle(self, color_list):
        self._line_color_cycle_colors = color_list
        self._line_color_cycle = itertools.cycle(color_list)

    def lineColorCycle(self):
        return list(self._line_color_cycle_colors)

    def addLegendItem(self, label, item):
        self._legend_items.append(item)
        self._legend_labels.append(label)

    def title(self):
        """ :rtype: str """
        return self._title if self._title is not None else "Unnamed"

    def setTitle(self, title):
        self._title = title

    def isUnnamed(self):
        return self._title is None

    def defaultStyle(self):
        style = PlotStyle("Default Style")
        style.copyStyleFrom(self._default_style)
        style.color = self.currentColor()
        return style

    def observationsStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("Observations Style")
        style.copyStyleFrom(self._observation_style)
        return style

    def refcaseStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("Refcase Style")
        style.copyStyleFrom(self._refcase_style)
        return style

    def historyStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("History Style")
        style.copyStyleFrom(self._history_style)
        return style

    def histogramStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("Histogram Style")
        style.copyStyleFrom(self._histogram_style)
        style.color = self.currentColor()
        return style

    def distributionStyle(self):
        """ @rtype: PlotStyle """
        style = PlotStyle("Distribution Style")
        style.copyStyleFrom(self._distribution_style)
        style.color = self.currentColor()
        return style

    def distributionLineStyle(self):
        """ @rtype: ert_gui.plottery.PlotStyle """
        style = PlotStyle("Distribution Line Style")
        style.copyStyleFrom(self._distribution_line_style)
        return style

    def xLabel(self):
        return self._x_label

    def yLabel(self):
        return self._y_label

    def legendItems(self):
        return self._legend_items

    def legendLabels(self):
        return self._legend_labels

    def setXLabel(self, label):
        self._x_label = label

    def setYLabel(self, label):
        self._y_label = label

    def setObservationsEnabled(self, enabled):
        self._observation_style.setEnabled(enabled)

    def isObservationsEnabled(self):
        return self._observation_style.isEnabled()

    def setRefcaseEnabled(self, enabled):
        self._refcase_style.setEnabled(enabled)

    def isRefcaseEnabled(self):
        return self._refcase_style.isEnabled()

    def setHistoryEnabled(self, enabled):
        self._history_style.setEnabled(enabled)

    def isHistoryEnabled(self):
        return self._history_style.isEnabled()

    def isLegendEnabled(self):
        return self._legend_enabled

    def isDistributionLineEnabled(self):
        return self._distribution_line_style.isEnabled()

    def setDistributionLineEnabled(self, enabled):
        self._distribution_line_style.setEnabled(enabled)

    def setLegendEnabled(self, enabled):
        self._legend_enabled = enabled

    def isGridEnabled(self):
        return self._grid_enabled

    def setGridEnabled(self, enabled):
        self._grid_enabled = enabled

    def setStatisticsStyle(self, statistic, style):
        """
        @type statistic: str
        @type style: PlotStyle
        """

        statistics_style = self._statistics_style[statistic]
        statistics_style.line_style = style.line_style
        statistics_style.marker = style.marker
        statistics_style.width = style.width
        statistics_style.size = style.size

    def getStatisticsStyle(self, statistic):
        style = self._statistics_style[statistic]
        copy_style = PlotStyle(style.name)
        copy_style.copyStyleFrom(style)
        copy_style.color = self.currentColor()
        return copy_style

    def setRefcaseStyle(self, style):
        """ @type style: PlotStyle """
        self._refcase_style.line_style = style.line_style
        self._refcase_style.marker = style.marker
        self._refcase_style.width = style.width
        self._refcase_style.size = style.size

    def setHistoryStyle(self, style):
        """ @type style: PlotStyle """
        self._history_style.line_style = style.line_style
        self._history_style.marker = style.marker
        self._history_style.width = style.width
        self._history_style.size = style.size


    def setDefaultStyle(self, style):
        """ @type style: PlotStyle """
        self._default_style.line_style = style.line_style
        self._default_style.marker = style.marker
        self._default_style.width = style.width
        self._default_style.size = style.size


    @property
    def limits(self):
        """ @rtype: PlotLimits """
        limits = PlotLimits()
        limits.copyLimitsFrom(self._limits)
        return limits

    @limits.setter
    def limits(self, value):
        """ @type value: PlotLimits """
        self._limits.copyLimitsFrom(value)


    def copyConfigFrom(self, other):
        """
        :type other: PlotConfig
        """
        self._default_style.copyStyleFrom(other._default_style, copy_enabled_state=True)
        self._refcase_style.copyStyleFrom(other._refcase_style, copy_enabled_state=True)
        self._history_style.copyStyleFrom(other._history_style, copy_enabled_state=True)
        self._histogram_style.copyStyleFrom(other._histogram_style, copy_enabled_state=True)
        self._observation_style.copyStyleFrom(other._observation_style, copy_enabled_state=True)
        self._distribution_style.copyStyleFrom(other._distribution_style, copy_enabled_state=True)
        self._distribution_line_style.copyStyleFrom(other._distribution_line_style, copy_enabled_state=True)

        self._statistics_style["mean"].copyStyleFrom(other._statistics_style["mean"], copy_enabled_state=True)
        self._statistics_style["p50"].copyStyleFrom(other._statistics_style["p50"], copy_enabled_state=True)
        self._statistics_style["min-max"].copyStyleFrom(other._statistics_style["min-max"], copy_enabled_state=True)
        self._statistics_style["p10-p90"].copyStyleFrom(other._statistics_style["p10-p90"], copy_enabled_state=True)
        self._statistics_style["p33-p67"].copyStyleFrom(other._statistics_style["p33-p67"], copy_enabled_state=True)
        self._statistics_style["std"].copyStyleFrom(other._statistics_style["std"], copy_enabled_state=True)

        self._legend_enabled = other._legend_enabled
        self._grid_enabled = other._grid_enabled

        self.setLineColorCycle(other._line_color_cycle_colors)

        self._legend_items = other._legend_items[:]
        self._legend_labels = other._legend_labels[:]

        self._x_label = other._x_label
        self._y_label = other._y_label

        self._limits.copyLimitsFrom(other._limits)

        if other._title is not None:
            self._title = other._title

    @classmethod
    def createCopy(cls, other):
        copy = PlotConfig(None)
        copy.copyConfigFrom(other)
        return copy

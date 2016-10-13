from matplotlib.patches import Rectangle
from matplotlib.lines import Line2D
from pandas import DataFrame
from .refcase import plotRefcase
from .observations import plotObservations
from .plot_tools import PlotTools


def plotStatistics(plot_context):
    """ @type plot_context: ert_gui.plottery.PlotContext """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    """:type: ert_gui.plotter.PlotConfig """
    axes = plot_context.figure().add_subplot(111)
    """:type: matplotlib.axes.Axes """

    plot_context.y_axis = plot_context.VALUE_AXIS
    plot_context.x_axis = plot_context.DATE_AXIS

    case_list = plot_context.cases()
    for case in case_list:
        data = plot_context.dataGatherer().gatherData(ert, case, key)

        if not data.empty:
            if not data.index.is_all_dates:
                plot_context.deactivateDateSupport()
                plot_context.x_axis = plot_context.INDEX_AXIS

            style = config.getStatisticsStyle("mean")
            rectangle = Rectangle((0, 0), 1, 1, color=style.color, alpha=0.8) # creates rectangle patch for legend use.
            config.addLegendItem(case, rectangle)

            statistics_data = DataFrame()
            std_dev_factor = config.getStandardDeviationFactor()

            statistics_data["Minimum"] = data.min(axis=1)
            statistics_data["Maximum"] = data.max(axis=1)
            statistics_data["Mean"] = data.mean(axis=1)
            statistics_data["p10"] = data.quantile(0.1, axis=1)
            statistics_data["p33"] = data.quantile(0.33, axis=1)
            statistics_data["p50"] = data.quantile(0.50, axis=1)
            statistics_data["p67"] = data.quantile(0.67, axis=1)
            statistics_data["p90"] = data.quantile(0.90, axis=1)
            std = data.std(axis=1) * std_dev_factor
            statistics_data["std+"] = statistics_data["Mean"] + std
            statistics_data["std-"] = statistics_data["Mean"] - std

            _plotPercentiles(axes, config, statistics_data, case)
            config.nextColor()

    _addStatisticsLegends(plot_config=config)

    plotRefcase(plot_context, axes)
    plotObservations(plot_context, axes)

    default_x_label = "Date" if plot_context.isDateSupportActive() else "Index"
    PlotTools.finalizePlot(plot_context, axes, default_x_label=default_x_label, default_y_label="Value")

def _addStatisticsLegends(plot_config):
    _addStatisticsLegend(plot_config, "mean")
    _addStatisticsLegend(plot_config, "p50")
    _addStatisticsLegend(plot_config, "min-max", 0.2)
    _addStatisticsLegend(plot_config, "p10-p90", 0.4)
    _addStatisticsLegend(plot_config, "std", 0.4)
    _addStatisticsLegend(plot_config, "p33-p67", 0.6)

def _addStatisticsLegend(plot_config, style_name, alpha_multiplier=1.0):
    style = plot_config.getStatisticsStyle(style_name)
    if style.isVisible():
        if style.line_style == "#":
            rectangle = Rectangle((0, 0), 1, 1, color='black', alpha=style.alpha * alpha_multiplier)  # creates rectangle patch for legend use.
            plot_config.addLegendItem(style.name, rectangle)
        else:
            line = Line2D([], [], color='black', marker=style.marker, linestyle=style.line_style, linewidth=style.width, alpha=style.alpha, markersize=style.size)
            plot_config.addLegendItem(style.name, line)


def _plotPercentiles(axes, plot_config, data, ensemble_label):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: ert_gui.plottery.PlotConfig
    @type data: DataFrame
    @type ensemble_label: Str
    """
    style = plot_config.getStatisticsStyle("mean")
    if style.isVisible():
        axes.plot(data.index.values, data["Mean"].values, alpha=style.alpha, linestyle=style.line_style, color=style.color, marker=style.marker, linewidth=style.width, markersize=style.size)

    style = plot_config.getStatisticsStyle("p50")
    if style.isVisible():
        axes.plot(data.index.values, data["p50"].values, alpha=style.alpha, linestyle=style.line_style, color=style.color, marker=style.marker, linewidth=style.width, markersize=style.size)

    style = plot_config.getStatisticsStyle("std")
    _plotPercentile(axes, style, data.index.values, data["std+"].values, data["std-"].values, 0.5)

    style = plot_config.getStatisticsStyle("min-max")
    _plotPercentile(axes, style, data.index.values, data["Maximum"].values, data["Minimum"].values, 0.5)

    style = plot_config.getStatisticsStyle("p10-p90")
    _plotPercentile(axes, style, data.index.values, data["p90"].values, data["p10"].values, 0.5)

    style = plot_config.getStatisticsStyle("p33-p67")
    _plotPercentile(axes, style, data.index.values, data["p67"].values, data["p33"].values, 0.5)


def _plotPercentile(axes, style, index_values, top_line_data, bottom_line_data, alpha_multiplier):
    alpha = style.alpha
    line_style = style.line_style
    color = style.color
    marker = style.marker

    if line_style == "#":
        axes.fill_between(index_values, bottom_line_data, top_line_data, alpha=alpha * alpha_multiplier, color=color)
    elif style.isVisible():
        axes.plot(index_values, bottom_line_data, alpha=alpha, linestyle=line_style, color=color, marker=marker, linewidth=style.width, markersize=style.size)
        axes.plot(index_values, top_line_data, alpha=alpha, linestyle=line_style, color=color, marker=marker, linewidth=style.width, markersize=style.size)

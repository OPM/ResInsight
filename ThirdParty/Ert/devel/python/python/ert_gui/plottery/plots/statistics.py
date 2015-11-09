from matplotlib.patches import Rectangle
from pandas import DataFrame
from .refcase import plotRefcase
from .observations import plotObservations
from .plot_tools import PlotTools


def plotStatistics(plot_context):
    """
    @type plot_context: PlotContext
    """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    axes = plot_context.figure().add_subplot(111)
    """:type: matplotlib.axes.Axes """

    case_list = plot_context.cases()
    for case in case_list:
        data = plot_context.dataGatherer().gatherData(ert, case, key)
        if not data.empty:
            if not data.index.is_all_dates:
                config.deactiveDateSupport()

            statistics_data = DataFrame()

            statistics_data["Minimum"] = data.min(axis=1)
            statistics_data["Maximum"] = data.max(axis=1)
            statistics_data["Mean"] = data.mean(axis=1)
            statistics_data["p10"] = data.quantile(0.1, axis=1)
            statistics_data["p33"] = data.quantile(0.33, axis=1)
            statistics_data["p50"] = data.quantile(0.50, axis=1)
            statistics_data["p67"] = data.quantile(0.67, axis=1)
            statistics_data["p90"] = data.quantile(0.90, axis=1)

            _plotPercentiles(axes, config, statistics_data, case)
            config.nextColor()

    plotRefcase(plot_context, axes)
    plotObservations(plot_context, axes)

    default_x_label = "Date" if config.isDateSupportActive() else "Index"
    PlotTools.finalizePlot(plot_context, axes, default_x_label=default_x_label, default_y_label="Value")


def _plotPercentiles(axes, plot_config, data, ensemble_label):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type ensemble_label: Str
    """

    line_color = plot_config.lineColor()
    line_alpha = plot_config.lineAlpha()

    minimum_line = axes.plot(data.index.values, data["Minimum"].values, alpha=line_alpha, linestyle="--", color=line_color)
    maximum_line = axes.plot(data.index.values, data["Maximum"].values, alpha=line_alpha, linestyle="--", color=line_color)
    p50_line = axes.plot(data.index.values, data["p50"].values, alpha=line_alpha, linestyle="--", color=line_color, marker="x")
    mean_line = axes.plot(data.index.values, data["Mean"].values, alpha=line_alpha, linestyle="-", color=line_color, marker="")
    axes.fill_between(data.index.values, data["p10"].values, data["p90"].values, alpha=line_alpha * 0.3, color=line_color)
    axes.fill_between(data.index.values, data["p33"].values, data["p67"].values, alpha=line_alpha * 0.5, color=line_color)

    rectangle_p10_p90 = Rectangle((0, 0), 1, 1, color=line_color, alpha=line_alpha * 0.3) # creates rectangle patch for legend use.
    rectangle_p33_p67 = Rectangle((0, 0), 1, 1, color=line_color, alpha=line_alpha * 0.5) # creates rectangle patch for legend use.

    plot_config.addLegendItem("Minimum/Maximum", minimum_line[0])
    plot_config.addLegendItem("P50", p50_line[0])
    plot_config.addLegendItem("Mean", mean_line[0])
    plot_config.addLegendItem("P10-P90", rectangle_p10_p90)
    plot_config.addLegendItem("P33-P67", rectangle_p33_p67)

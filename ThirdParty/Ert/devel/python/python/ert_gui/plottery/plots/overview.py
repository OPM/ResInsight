from matplotlib.patches import Rectangle
from pandas import DataFrame
from .refcase import plotRefcase
from .observations import plotObservations
from .plot_tools import PlotTools


def plotOverview(plot_context):
    """
    @type plot_context: ert_gui.plottery.PlotContext
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

            min_max_data = DataFrame()
            min_max_data["Minimum"] = data.min(axis=1)
            min_max_data["Maximum"] = data.max(axis=1)

            _plotArea(axes, config, min_max_data, case)
            config.nextColor()

    plotRefcase(plot_context, axes)
    plotObservations(plot_context, axes)

    default_x_label = "Date" if config.isDateSupportActive() else "Index"
    PlotTools.finalizePlot(plot_context, axes, default_x_label=default_x_label, default_y_label="Value")


def _plotArea(axes, plot_config, data, ensemble_label):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type ensemble_label: Str
    """

    line_color = plot_config.lineColor()
    line_alpha = plot_config.lineAlpha() * 0.5

    poly_collection = axes.fill_between(data.index.values, data["Minimum"].values, data["Maximum"].values, alpha=line_alpha, color=line_color)

    rectangle = Rectangle((0, 0), 1, 1, color=line_color) # creates rectangle patch for legend use.

    plot_config.addLegendItem(ensemble_label, rectangle)
from .refcase import plotRefcase
from .history import plotHistory
from .observations import plotObservations
from .plot_tools import PlotTools

def plotEnsemble(plot_context):
    """
    @type plot_context: ert_gui.plottery.PlotContext
    """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    """:type: ert_gui.plottery.PlotConfig """
    axes = plot_context.figure().add_subplot(111)
    """:type: matplotlib.axes.Axes """

    case_list = plot_context.cases()

    plot_context.y_axis = plot_context.VALUE_AXIS
    plot_context.x_axis = plot_context.DATE_AXIS

    for case in case_list:
        data = plot_context.dataGatherer().gatherData(ert, case, key)
        if not data.empty:
            if not data.index.is_all_dates:
                plot_context.deactivateDateSupport()
                plot_context.x_axis = plot_context.INDEX_AXIS

            _plotLines(axes, config, data, case, plot_context.isDateSupportActive())
            config.nextColor()

    plotRefcase(plot_context, axes)
    plotObservations(plot_context, axes)
    plotHistory(plot_context, axes)

    default_x_label = "Date" if plot_context.isDateSupportActive() else "Index"
    PlotTools.finalizePlot(plot_context, axes, default_x_label=default_x_label, default_y_label="Value")


def _plotLines(axes, plot_config, data, ensemble_label, is_date_supported):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: ert_gui.plottery.PlotConfig
    @type data: pandas.DataFrame
    @type ensemble_label: Str
    """

    style = plot_config.defaultStyle()

    if is_date_supported:
        lines = axes.plot_date(x=data.index.values, y=data, color=style.color, alpha=style.alpha, marker=style.marker, linestyle=style.line_style, linewidth=style.width, markersize=style.size)
    else:
        lines = axes.plot(data.index.values, data, color=style.color, alpha=style.alpha, marker=style.marker, linestyle=style.line_style, linewidth=style.width, markersize=style.size)

    if len(lines) > 0:
        plot_config.addLegendItem(ensemble_label, lines[0])

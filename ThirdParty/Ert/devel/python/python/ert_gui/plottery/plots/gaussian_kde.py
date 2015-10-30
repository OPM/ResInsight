import numpy
from scipy.stats import gaussian_kde
from .plot_tools import PlotTools


def plotGaussianKDE(plot_context):
    """
    @type plot_context: PlotContext
    """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    axes = plot_context.figure().add_subplot(111)
    """:type: matplotlib.axes.Axes """

    config.deactiveDateSupport()

    if key.startswith("LOG10_"):
        key = key[6:]
        axes.set_xscale("log")

    case_list = plot_context.cases()
    for case in case_list:
        data = plot_context.dataGatherer().gatherData(ert, case, key)

        if not data.empty and data.nunique() > 1:
            _plotGaussianKDE(axes, config, data, case)
            config.nextColor()

    PlotTools.finalizePlot(plot_context, axes, default_x_label="Value", default_y_label="Density")


def _plotGaussianKDE(axes, plot_config, data, label):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type label: Str
    """

    # axes.set_xlabel(plot_config.xLabel())
    # axes.set_ylabel(plot_config.yLabel())

    line_color = plot_config.lineColor()
    line_alpha = plot_config.lineAlpha()
    line_marker = plot_config.lineMarker()
    line_style = plot_config.lineStyle()
    line_width = 2

    if data.dtype == "object":
        data = data.convert_objects(convert_numeric=True)

    if data.dtype == "object":
        pass

    else:
        sample_range = data.max() - data.min()
        indexes = numpy.linspace(data.min() - 0.5 * sample_range, data.max() + 0.5 * sample_range, 1000)
        gkde = gaussian_kde(data.values)
        evaluated_gkde = gkde.evaluate(indexes)

        lines = axes.plot(indexes, evaluated_gkde, linewidth=line_width, color=line_color, alpha=line_alpha)

        if len(lines) > 0:
            plot_config.addLegendItem(label, lines[0])
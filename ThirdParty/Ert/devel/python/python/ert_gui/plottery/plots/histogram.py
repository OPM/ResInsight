from math import sqrt, ceil, floor, log10
from matplotlib.patches import Rectangle
import numpy
from .plot_tools import PlotTools



def plotHistogram(plot_context):
    """
    @type plot_context: ert_gui.plottery.PlotContext
    """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    case_list = plot_context.cases()
    case_count = len(case_list)

    if config.xLabel() is None:
        config.setXLabel("Value")

    if config.yLabel() is None:
        config.setYLabel("Count")

    use_log_scale = False
    if key.startswith("LOG10_"):
        key = key[6:]
        use_log_scale = True

    data = {}
    minimum = None
    maximum = None
    for case in case_list:
        data[case] = plot_context.dataGatherer().gatherData(ert, case, key)

        if minimum is None:
            minimum = data[case].min()
        else:
            minimum = min(minimum, data[case].min())

        if maximum is None:
            maximum = data[case].max()
        else:
            maximum = max(maximum, data[case].max())

        #todo: bin count must also be determined across all cases...

    axes = {}
    """:type: dict of (str, matplotlib.axes.Axes) """
    for index, case in enumerate(case_list):
        axes[case] = plot_context.figure().add_subplot(case_count, 1, index + 1)

        axes[case].set_title("%s (%s)" % (config.title(), case))

        if use_log_scale:
            axes[case].set_xscale("log")

        if not data[case].empty:
            _plotHistogram(axes[case], config, data[case], case, use_log_scale, minimum, maximum)

            config.nextColor()
            PlotTools.showGrid(axes[case], plot_context)

    max_count = max([subplot.get_ylim()[1] for subplot in axes.values()])

    for subplot in axes.values():
        subplot.set_ylim(0, max_count)


def _plotHistogram(axes, plot_config, data, label, use_log_scale=False, minimum=None, maximum=None):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type label: Str
    """

    axes.set_xlabel(plot_config.xLabel())
    axes.set_ylabel(plot_config.yLabel())

    line_color = plot_config.lineColor()
    line_alpha = plot_config.lineAlpha()
    line_marker = plot_config.lineMarker()
    line_style = plot_config.lineStyle()
    line_width = 2

    if data.dtype == "object":
        data = data.convert_objects(convert_numeric=True)

    if data.dtype == "object":
        counts = data.value_counts()
        x = counts.index.values
        freq = counts.values
        pos = numpy.arange(len(x))
        width = 1.0
        axes.set_xticks(pos + (width / 2.0))
        axes.set_xticklabels(x)
        axes.bar(pos, freq, alpha=line_alpha, color=line_color, width=width)
    else:
        bins = int(ceil(sqrt(len(data.index))))

        if use_log_scale:
            bins = _histogramLogBins(data, bins, minimum, maximum)
        elif minimum is not None and maximum is not None:
            bins = numpy.linspace(minimum, maximum, bins)

        axes.hist(data.values, alpha=line_alpha, bins=bins, color=line_color)

    rectangle = Rectangle((0, 0), 1, 1, color=line_color) # creates rectangle patch for legend use.'
    plot_config.addLegendItem(label, rectangle)



def _histogramLogBins(data, bin_count, minimum=None, maximum=None):
    """
    @type data: pandas.DataFrame
    @rtype: int
    """

    if minimum is None:
        minimum = data.min()

    if maximum is None:
        maximum = data.max()

    minimum = log10(float(minimum))
    maximum = log10(float(maximum))

    min_value = int(floor(minimum))
    max_value = int(ceil(maximum))

    log_bin_count = max_value - min_value

    if log_bin_count < bin_count:
        next_bin_count = log_bin_count * 2

        if bin_count - log_bin_count > next_bin_count - bin_count:
            log_bin_count = next_bin_count
        else:
            log_bin_count = bin_count

    return 10 ** numpy.linspace(minimum, maximum, log_bin_count)
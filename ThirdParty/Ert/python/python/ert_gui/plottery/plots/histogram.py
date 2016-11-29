from math import sqrt, ceil, floor, log10
from matplotlib.patches import Rectangle
import numpy
from .plot_tools import PlotTools
import pandas as pd

def plotHistogram(plot_context):
    """ @type plot_context: ert_gui.plottery.PlotContext """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()

    case_list = plot_context.cases()
    case_count = len(case_list)

    plot_context.x_axis = plot_context.VALUE_AXIS
    plot_context.Y_axis = plot_context.COUNT_AXIS

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
    categories = set()
    max_element_count = 0
    categorical = False
    for case in case_list:
        data[case] = plot_context.dataGatherer().gatherData(ert, case, key)

        if data[case].dtype == "object":
            try:
                data[case] = pd.to_numeric(data[case], errors='ignore')
            except AttributeError:
                data[case] = data[case].convert_objects(convert_numeric=True)

        if data[case].dtype == "object":
            categorical = True

        if categorical:
            categories = categories.union(set(data[case].unique()))
        else:
            if minimum is None:
                minimum = data[case].min()
            else:
                minimum = min(minimum, data[case].min())

            if maximum is None:
                maximum = data[case].max()
            else:
                maximum = max(maximum, data[case].max())

            max_element_count = max(max_element_count, len(data[case].index))

    categories = sorted(categories)
    bin_count = int(ceil(sqrt(max_element_count)))

    axes = {}
    """:type: dict of (str, matplotlib.axes.Axes) """
    for index, case in enumerate(case_list):
        axes[case] = plot_context.figure().add_subplot(case_count, 1, index + 1)

        axes[case].set_title("%s (%s)" % (config.title(), case))

        if use_log_scale:
            axes[case].set_xscale("log")

        if not data[case].empty:
            if categorical:
                _plotCategoricalHistogram(axes[case], config, data[case], case, categories)
            else:
                _plotHistogram(axes[case], config, data[case], case, bin_count, use_log_scale, minimum, maximum)

            config.nextColor()
            PlotTools.showGrid(axes[case], plot_context)

    min_count = 0
    max_count = max([subplot.get_ylim()[1] for subplot in axes.values()])

    custom_limits = plot_context.plotConfig().limits

    if custom_limits.count_maximum is not None:
        max_count = custom_limits.count_maximum

    if custom_limits.count_minimum is not None:
        min_count = custom_limits.count_minimum

    for subplot in axes.values():
        subplot.set_ylim(min_count, max_count)
        subplot.set_xlim(custom_limits.value_minimum, custom_limits.value_maximum)


def _plotCategoricalHistogram(axes, plot_config, data, label, categories):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type label: str
    @type categories: list of str
    """

    axes.set_xlabel(plot_config.xLabel())
    axes.set_ylabel(plot_config.yLabel())

    style = plot_config.histogramStyle()

    counts = data.value_counts()
    freq = [counts[category] if category in counts else 0 for category in categories]
    pos = numpy.arange(len(categories))
    width = 1.0
    axes.set_xticks(pos + (width / 2.0))
    axes.set_xticklabels(categories)

    axes.bar(pos, freq, alpha=style.alpha, color=style.color, width=width)

    rectangle = Rectangle((0, 0), 1, 1, color=style.color) # creates rectangle patch for legend use.
    plot_config.addLegendItem(label, rectangle)


def _plotHistogram(axes, plot_config, data, label, bin_count, use_log_scale=False, minimum=None, maximum=None):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type label: str
    """

    axes.set_xlabel(plot_config.xLabel())
    axes.set_ylabel(plot_config.yLabel())

    style = plot_config.histogramStyle()

    if minimum is not None and maximum is not None:
        if use_log_scale:
            bins = _histogramLogBins(bin_count, minimum, maximum)
        else:
            bins = numpy.linspace(minimum, maximum, bin_count)
    else:
        bins = bin_count

    axes.hist(data.values, alpha=style.alpha, bins=bins, color=style.color)

    if minimum == maximum:
        minimum -= 0.5
        maximum += 0.5

    axes.set_xlim(minimum, maximum)

    rectangle = Rectangle((0, 0), 1, 1, color=style.color) # creates rectangle patch for legend use.'
    plot_config.addLegendItem(label, rectangle)



def _histogramLogBins(bin_count, minimum=None, maximum=None):
    """
    @type data: pandas.DataFrame
    @rtype: int
    """
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

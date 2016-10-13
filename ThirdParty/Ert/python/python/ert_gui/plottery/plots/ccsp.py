from matplotlib.patches import Rectangle
from matplotlib.lines import Line2D
from .plot_tools import PlotTools
import pandas as pd

def plotCrossCaseStatistics(plot_context):
    """ @type plot_context: ert_gui.plottery.PlotContext """
    ert = plot_context.ert()
    key = plot_context.key()
    config = plot_context.plotConfig()
    axes = plot_context.figure().add_subplot(111)
    """:type: matplotlib.axes.Axes """

    plot_context.deactivateDateSupport()

    plot_context.y_axis = plot_context.VALUE_AXIS

    if key.startswith("LOG10_"):
        key = key[6:]
        axes.set_yscale("log")

    case_list = plot_context.cases()
    case_indexes = []
    ccs = {
        "index": [],
        "mean": {},
        "min": {},
        "max": {},
        "std": {},
        "p10": {},
        "p33": {},
        "p50": {},
        "p67": {},
        "p90": {}
    }
    for case_index, case in enumerate(case_list):
        case_indexes.append(case_index)
        data = plot_context.dataGatherer().gatherData(ert, case, key)
        std_dev_factor = config.getStandardDeviationFactor()

        if not data.empty:
            data = _assertNumeric(data)
            if not data is None:
                ccs["index"].append(case_index)
                ccs["mean"][case_index] = data.mean()
                ccs["min"][case_index] = data.min()
                ccs["max"][case_index] = data.max()
                ccs["std"][case_index] = data.std() * std_dev_factor
                ccs["p10"][case_index] = data.quantile(0.1)
                ccs["p33"][case_index] = data.quantile(0.33)
                ccs["p50"][case_index] = data.quantile(0.5)
                ccs["p67"][case_index] = data.quantile(0.67)
                ccs["p90"][case_index] = data.quantile(0.9)

                _plotCrossCaseStatistics(axes, config, ccs, case_index)
                config.nextColor()

    if config.isDistributionLineEnabled() and len(ccs["index"]) > 1:
        _plotConnectionLines(axes, config, ccs)

    _addStatisticsLegends(config)

    axes.set_xticks([-1] + case_indexes + [len(case_indexes)])

    rotation = 0
    if len(case_list) > 3:
        rotation = 30

    axes.set_xticklabels([""] + case_list + [""], rotation=rotation)

    PlotTools.finalizePlot(plot_context, axes, default_x_label="Case", default_y_label="Value")


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
            line = Line2D([], [], color='black', marker=style.marker, linestyle=style.line_style, linewidth=style.width, alpha=style.alpha)
            plot_config.addLegendItem(style.name, line)


def _assertNumeric(data):
    if data.dtype == "object":
        try:
            data = pd.to_numeric(data, errors='coerce')
        except AttributeError:
            data = data.convert_objects(convert_numeric=True)

    if data.dtype == "object":
        data = None
    return data


def _plotCrossCaseStatistics(axes, plot_config, data, index):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    @type index: int
    """

    axes.set_xlabel(plot_config.xLabel())
    axes.set_ylabel(plot_config.yLabel())

    style = plot_config.getStatisticsStyle("mean")
    if style.isVisible():
        axes.plot([index], data["mean"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)

    style = plot_config.getStatisticsStyle("p50")
    if style.isVisible():
        axes.plot([index], data["p50"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)

    style = plot_config.getStatisticsStyle("std")
    if style.isVisible():
        axes.plot([index], data["mean"][index] + data["std"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)
        axes.plot([index], data["mean"][index] - data["std"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)

    style = plot_config.getStatisticsStyle("min-max")
    if style.isVisible():
        axes.plot([index], data["min"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)
        axes.plot([index], data["max"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)

    style = plot_config.getStatisticsStyle("p10-p90")
    if style.isVisible():
        axes.plot([index], data["p10"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)
        axes.plot([index], data["p90"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)

    style = plot_config.getStatisticsStyle("p33-p67")
    if style.isVisible():
        axes.plot([index], data["p33"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)
        axes.plot([index], data["p67"][index], alpha=style.alpha, linestyle="", color=style.color, marker=style.marker, markersize=style.size)


def _plotConnectionLines(axes, plot_config, ccs):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type ccs: dict[str, dict[int, float]]
    """
    line_style = plot_config.distributionLineStyle()
    index_list = ccs["index"]
    """ :type: list[int] """
    for index in range(len(index_list) - 1):
        from_index = index_list[index]
        to_index = index_list[index + 1]

        x = [from_index, to_index]

        style = plot_config.getStatisticsStyle("mean")
        if style.isVisible():
            y = [ccs["mean"][from_index], ccs["mean"][to_index]]
            axes.plot(x, y, alpha=line_style.alpha, linestyle=style.line_style, color=line_style.color, linewidth=style.width)

        style = plot_config.getStatisticsStyle("p50")
        if style.isVisible():
            y = [ccs["p50"][from_index], ccs["p50"][to_index]]
            axes.plot(x, y, alpha=line_style.alpha, linestyle=style.line_style, color=line_style.color, linewidth=style.width)

        style = plot_config.getStatisticsStyle("std")
        if style.isVisible():
            mean = [ccs["mean"][from_index], ccs["mean"][to_index]]
            std = [ccs["std"][from_index], ccs["std"][to_index]]

            y_1 = [mean[0] + std[0], mean[1] + std[1]]
            y_2 = [mean[0] - std[0], mean[1] - std[1]]

            linestyle = style.line_style
            if linestyle == "#":
                linestyle = ""

            axes.plot(x, y_1, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)
            axes.plot(x, y_2, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)

        style = plot_config.getStatisticsStyle("min-max")
        if style.isVisible():
            y_1 = [ccs["min"][from_index], ccs["min"][to_index]]
            y_2 = [ccs["max"][from_index], ccs["max"][to_index]]

            linestyle = style.line_style
            if linestyle == "#":
                linestyle = ""

            axes.plot(x, y_1, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)
            axes.plot(x, y_2, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)

        style = plot_config.getStatisticsStyle("p10-p90")
        if style.isVisible():
            y_1 = [ccs["p10"][from_index], ccs["p10"][to_index]]
            y_2 = [ccs["p90"][from_index], ccs["p90"][to_index]]

            linestyle = style.line_style
            if linestyle == "#":
                linestyle = ""

            axes.plot(x, y_1, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)
            axes.plot(x, y_2, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)

        style = plot_config.getStatisticsStyle("p33-p67")
        if style.isVisible():
            y_1 = [ccs["p33"][from_index], ccs["p33"][to_index]]
            y_2 = [ccs["p67"][from_index], ccs["p67"][to_index]]

            linestyle = style.line_style
            if linestyle == "#":
                linestyle = ""

            axes.plot(x, y_1, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)
            axes.plot(x, y_2, alpha=style.alpha, linestyle=linestyle, color=line_style.color, linewidth=style.width)


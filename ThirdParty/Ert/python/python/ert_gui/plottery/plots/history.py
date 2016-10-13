def plotHistory(plot_context, axes):
    ert = plot_context.ert()
    key = plot_context.key()

    if len(plot_context.cases()) == 0:
        case = None
    else:
        case = plot_context.cases()[0]

    config = plot_context.plotConfig()
    data_gatherer = plot_context.dataGatherer()

    if config.isHistoryEnabled() and data_gatherer.hasHistoryGatherFunction():
        history_data = data_gatherer.gatherHistoryData(ert, case, key)

        if not history_data.empty:
            _plotHistory(axes, config, history_data)


def _plotHistory(axes, plot_config, data):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    """

    style = plot_config.historyStyle()

    lines = axes.plot_date(x=data.index.values, y=data, color=style.color, alpha=style.alpha, marker=style.marker, linestyle=style.line_style,
                           linewidth=style.width, markersize=style.size)

    if len(lines) > 0 and style.isVisible():
        plot_config.addLegendItem("History", lines[0])

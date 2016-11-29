def plotRefcase(plot_context, axes):
   ert = plot_context.ert()
   key = plot_context.key()
   config = plot_context.plotConfig()
   data_gatherer = plot_context.dataGatherer()

   if config.isRefcaseEnabled() and data_gatherer.hasRefcaseGatherFunction():
       refcase_data = data_gatherer.gatherRefcaseData(ert, key)

       if not refcase_data.empty:
           _plotRefcase(axes, config, refcase_data)


def _plotRefcase(axes, plot_config, data):
    """
    @type axes: matplotlib.axes.Axes
    @type plot_config: PlotConfig
    @type data: DataFrame
    """

    style = plot_config.refcaseStyle()

    lines = axes.plot_date(x=data.index.values, y=data, color=style.color, alpha=style.alpha, marker=style.marker, linestyle=style.line_style, linewidth=style.width, markersize=style.size)

    if len(lines) > 0 and style.isVisible():
        plot_config.addLegendItem("Refcase", lines[0])
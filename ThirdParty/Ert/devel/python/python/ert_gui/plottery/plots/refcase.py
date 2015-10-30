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

    line_color = plot_config.refcaseColor()
    line_alpha = plot_config.refcaseAlpha()
    line_marker = plot_config.refcaseMarker()
    line_style = plot_config.refcaseStyle()
    line_width = plot_config.refcaseWidth()

    lines = axes.plot_date(x=data.index.values, y=data, color=line_color, alpha=line_alpha, marker=line_marker, linestyle=line_style, linewidth=line_width)

    if len(lines) > 0:
        plot_config.addLegendItem("Refcase", lines[0])
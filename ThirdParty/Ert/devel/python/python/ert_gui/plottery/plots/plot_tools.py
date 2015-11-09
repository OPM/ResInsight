class PlotTools(object):
    @staticmethod
    def showGrid(axes, plot_context):
        config = plot_context.plotConfig()
        if config.isGridEnabled():
            axes.grid()


    @staticmethod
    def showLegend(axes, plot_context):
        config = plot_context.plotConfig()
        if config.isLegendEnabled() and len(config.legendItems()) > 0:
            axes.legend(config.legendItems(), config.legendLabels())


    @staticmethod
    def finalizePlot(plot_context, axes, default_x_label="Unnamed", default_y_label="Unnamed"):
        PlotTools.showLegend(axes, plot_context)
        PlotTools.showGrid(axes, plot_context)

        PlotTools.__setupLabels(plot_context, default_x_label, default_y_label)

        plot_config = plot_context.plotConfig()
        axes.set_xlabel(plot_config.xLabel())
        axes.set_ylabel(plot_config.yLabel())

        axes.set_title(plot_config.title())

        if plot_config.isDateSupportActive():
            plot_context.figure().autofmt_xdate()


    @staticmethod
    def __setupLabels(plot_context, default_x_label, default_y_label):
        ert = plot_context.ert()
        key = plot_context.key()
        config = plot_context.plotConfig()

        if config.xLabel() is None:
            config.setXLabel(default_x_label)

        if config.yLabel() is None:
            config.setYLabel(default_y_label)

            if ert.eclConfig().hasRefcase() and key in ert.eclConfig().getRefcase():
                unit = ert.eclConfig().getRefcase().unit(key)
                if unit != "":
                    config.setYLabel(unit)
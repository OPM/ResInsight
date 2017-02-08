from ert_gui.plottery import PlotConfig


class PlotConfigFactory(object):

    @classmethod
    def createPlotConfigForKey(cls, ert, key):
        """
        @type ert: ert.enkf.enkf_main.EnKFMain
        @param key: str
        @return: PlotConfig
        """
        plot_config = PlotConfig(ert.plotConfig() , title = key)
        return PlotConfigFactory.updatePlotConfigForKey(ert, key, plot_config)


    @classmethod
    def updatePlotConfigForKey(cls, ert, key, plot_config):
        """
        @type ert: ert.enkf.enkf_main.EnKFMain
        @param key: str
        @return: PlotConfig
        """
        key_manager = ert.getKeyManager()
        # The styling of statistics changes based on the nature of the data
        if key_manager.isSummaryKey(key) or key_manager.isGenDataKey(key):
            mean_style = plot_config.getStatisticsStyle("mean")
            mean_style.line_style = "-"
            plot_config.setStatisticsStyle("mean", mean_style)

            p10p90_style = plot_config.getStatisticsStyle("p10-p90")
            p10p90_style.line_style = "--"
            plot_config.setStatisticsStyle("p10-p90", p10p90_style)
        else:
            mean_style = plot_config.getStatisticsStyle("mean")
            mean_style.line_style = "-"
            mean_style.marker = "o"
            plot_config.setStatisticsStyle("mean", mean_style)

            std_style = plot_config.getStatisticsStyle("std")
            std_style.line_style = "--"
            std_style.marker = "D"
            plot_config.setStatisticsStyle("std", std_style)

        return plot_config

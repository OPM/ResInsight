from .plot_config import PlotConfig
from .plot_data_gatherer import PlotDataGatherer

class PlotContext(object):

    def __init__(self, ert, figure, plot_config, cases, key, data_gatherer):
        super(PlotContext, self).__init__()
        self.__data_gatherer = data_gatherer
        self.__key = key
        self.__cases = cases
        self.__figure = figure
        self.__ert = ert
        self.__plot_config = plot_config

    def figure(self):
        """ :rtype: matplotlib.figure.Figure"""
        return self.__figure

    def plotConfig(self):
        """ :rtype: PlotConfig """
        return self.__plot_config

    def ert(self):
        """ :rtype: ert.enkf.EnKFMain"""
        return self.__ert

    def cases(self):
        """ :rtype: list of str """
        return self.__cases

    def key(self):
        """ :rtype: str """
        return self.__key

    def dataGatherer(self):
        """ :rtype: PlotDataGatherer """
        return self.__data_gatherer



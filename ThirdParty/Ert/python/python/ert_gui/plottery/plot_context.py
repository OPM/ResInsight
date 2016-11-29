from .plot_config import PlotConfig
from .plot_data_gatherer import PlotDataGatherer

class PlotContext(object):
    UNKNOWN_AXIS = None
    VALUE_AXIS = "VALUE"
    DATE_AXIS = "DATE"
    INDEX_AXIS = "INDEX"
    COUNT_AXIS = "COUNT"
    DENSITY_AXIS = "DENSITY"
    DEPTH_AXIS = "DEPTH"
    AXIS_TYPES = [UNKNOWN_AXIS, COUNT_AXIS, DATE_AXIS, DENSITY_AXIS, DEPTH_AXIS, INDEX_AXIS, VALUE_AXIS]

    def __init__(self, ert, figure, plot_config, cases, key, data_gatherer):
        super(PlotContext, self).__init__()
        self._data_gatherer = data_gatherer
        self._key = key
        self._cases = cases
        self._figure = figure
        self._ert = ert
        self._plot_config = plot_config

        self._date_support_active = True
        self._x_axis = None
        self._y_axis = None

    def figure(self):
        """ :rtype: matplotlib.figure.Figure"""
        return self._figure

    def plotConfig(self):
        """ :rtype: PlotConfig """
        return self._plot_config

    def ert(self):
        """ :rtype: ert.enkf.EnKFMain"""
        return self._ert

    def cases(self):
        """ :rtype: list of str """
        return self._cases

    def key(self):
        """ :rtype: str """
        return self._key

    def dataGatherer(self):
        """ :rtype: PlotDataGatherer """
        return self._data_gatherer

    def deactivateDateSupport(self):
        self._date_support_active = False

    def isDateSupportActive(self):
        """ @rtype: bool """
        return self._date_support_active

    @property
    def x_axis(self):
        """ @rtype: str """
        return self._x_axis

    @x_axis.setter
    def x_axis(self, value):
        """ @type value: str """
        if not value in PlotContext.AXIS_TYPES:
            raise UserWarning("Axis: '%s' is not one of: %s" % (value, PlotContext.AXIS_TYPES))
        self._x_axis = value

    @property
    def y_axis(self):
        """ @rtype: str """
        return self._y_axis

    @y_axis.setter
    def y_axis(self, value):
        """ @type value: str """
        if not value in PlotContext.AXIS_TYPES:
            raise UserWarning("Axis: '%s' is not one of: %s" % (value, PlotContext.AXIS_TYPES))
        self._y_axis = value
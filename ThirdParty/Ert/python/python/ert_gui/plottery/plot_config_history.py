from ert_gui.plottery import PlotConfig


class PlotConfigHistory(object):
    """ A Class for tracking changes to a PlotConfig class (supports undo, redo and reset)"""

    def __init__(self, name, initial):
        super(PlotConfigHistory, self).__init__()
        self._name = name
        self._initial = PlotConfig.createCopy(initial)
        self._undo_history = []
        self._redo_history = []
        self._current = PlotConfig.createCopy(self._initial)

    def isUndoPossible(self):
        """ @rtype: bool """
        return len(self._undo_history) > 0

    def isRedoPossible(self):
        """ @rtype: bool """
        return len(self._redo_history) > 0

    def applyChanges(self, plot_config):
        """ @type plot_config: PlotConfig """
        self._undo_history.append(self._current)
        copy = PlotConfig.createCopy(self._current)
        copy.copyConfigFrom(plot_config)
        self._current = copy
        del self._redo_history[:]

    def resetChanges(self):
        self.applyChanges(self._initial)

    def undoChanges(self):
        if self.isUndoPossible():
            self._redo_history.append(self._current)
            self._current = self._undo_history.pop()

    def redoChanges(self):
        if self.isRedoPossible():
            self._undo_history.append(self._current)
            self._current = self._redo_history.pop()

    def getPlotConfig(self):
        """ @rtype: PlotConfig """
        return PlotConfig.createCopy(self._current)

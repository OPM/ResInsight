from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget


class SimulationConfigPanel(QWidget):

    simulationConfigurationChanged = pyqtSignal()

    def __init__(self, simulation_model, advanced_option=False):
        QWidget.__init__(self)
        self.setContentsMargins(10, 10, 10, 10)
        self.__simulation_model = simulation_model
        self._advanced_option = advanced_option

    @property
    def is_advanced_option(self):
        return self._advanced_option

    def getSimulationModel(self):
        return self.__simulation_model

    def isConfigurationValid(self):
        return True

    def toggleAdvancedOptions(self, show_advanced):
        raise NotImplementedError("toggleAdvancedOptions must be implemented!")

    def getSimulationArguments(self):
        """" @rtype: dict[str, object]"""
        return {}

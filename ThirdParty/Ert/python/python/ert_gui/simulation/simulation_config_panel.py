from PyQt4.QtCore import pyqtSignal
from PyQt4.QtGui import QWidget


class SimulationConfigPanel(QWidget):

    simulationConfigurationChanged = pyqtSignal()

    def __init__(self, simulation_model):
        QWidget.__init__(self)
        self.setContentsMargins(10, 10, 10, 10)
        self.__simulation_model = simulation_model

    def getSimulationModel(self):
        return self.__simulation_model

    def isConfigurationValid(self):
        return True

    def toggleAdvancedOptions(self, show_advanced):
        raise NotImplementedError("toggleAdvancedOptions must be implemented!")

    def getSimulationArguments(self):
        """" @rtype: dict[str, object]"""
        return {}

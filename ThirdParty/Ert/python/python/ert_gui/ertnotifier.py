from ert.enkf import EnKFMain
from PyQt4.QtCore import QObject, pyqtSignal, pyqtSlot

class ErtNotifier(QObject):
    ertChanged = pyqtSignal()

    def __init__(self, ert, config_file, parent=None):
        QObject.__init__(self, parent)
        self._ert = ert
        self._config_file = config_file

    def _checkErt(self):
        if self._ert is None:
            raise ValueError("Ert is undefined.")

    @property
    def ert(self):
        """ @rtype: EnKFMain """
        self._checkErt()
        return self._ert


    @property
    def config_file(self):
        """ @rtype: str """
        self._checkErt()
        return self._config_file

    pyqtSlot()
    def emitErtChange(self):
        self._checkErt()
        self.ertChanged.emit()

    pyqtSignal()
    def reloadERT(self, config_file):
        import sys
        import os
        python_executable = sys.executable
        ert_gui_main = sys.argv[0]

        self._ert.free()
        os.execl(python_executable, python_executable, ert_gui_main, config_file)


ERT = ErtNotifier(None, None)
""" @type: ErtNotifier """

def configureErtNotifier(ert, config_file):
    ERT._ert = ert
    ERT._config_file = config_file

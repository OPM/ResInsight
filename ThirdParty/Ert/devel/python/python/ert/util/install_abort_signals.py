from ert.util import UTIL_LIB
from ert.cwrap import CWrapper

def installAbortSignals():
    install_signals()

    
cwrapper = CWrapper(UTIL_LIB)
install_signals = cwrapper.prototype("void util_install_signals()")

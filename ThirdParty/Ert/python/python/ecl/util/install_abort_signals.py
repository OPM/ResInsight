from ecl.util import UtilPrototype


def installAbortSignals():
    install_signals()


def updateAbortSignals():
    """
    Will install the util_abort_signal for all UNMODIFIED signals.
    """
    update_signals()


install_signals = UtilPrototype("void util_install_signals()")
update_signals = UtilPrototype("void util_update_signals()")

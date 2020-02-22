from ecl import EclPrototype


def installAbortSignals():
    install_signals()


def updateAbortSignals():
    """
    Will install the util_abort_signal for all UNMODIFIED signals.
    """
    update_signals()


install_signals = EclPrototype("void util_install_signals()")
update_signals = EclPrototype("void util_update_signals()")

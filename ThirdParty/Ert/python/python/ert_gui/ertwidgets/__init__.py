from PyQt4 import QtGui, QtCore

img_prefix = ""

def addHelpToWidget(widget, link):
    original_enter_event = widget.enterEvent

    def enterEvent(event):
        original_enter_event(event)
        try:
            from ert_gui.tools import HelpCenter
            HelpCenter.getHelpCenter("ERT").setHelpMessageLink(link)
        except AttributeError:
            pass

    widget.enterEvent = enterEvent


def showWaitCursorWhileWaiting(func):
    """A function decorator to show the wait cursor while the function is working."""

    def wrapper(*arg):
        QtGui.QApplication.setOverrideCursor(QtGui.QCursor(QtCore.Qt.WaitCursor))
        try:
            res = func(*arg)
            return res
        except:
            raise
        finally:
            QtGui.QApplication.restoreOverrideCursor()

    return wrapper


def resourceIcon(name):
    """Load an image as an icon"""
    # print("Icon used: %s" % name)
    return QtGui.QIcon(img_prefix + name)


def resourceStateIcon(on, off):
    """Load two images as an icon with on and off states"""
    icon = QtGui.QIcon()
    icon.addPixmap(resourceImage(on), state=QtGui.QIcon.On)
    icon.addPixmap(resourceImage(off), state=QtGui.QIcon.Off)
    return icon


def resourceImage(name):
    """Load an image as a Pixmap"""
    return QtGui.QPixmap(img_prefix + name)


def resourceMovie(name):
    """ @rtype: QMovie """
    movie = QtGui.QMovie(img_prefix + name)
    movie.start()
    return movie


from .legend import Legend
from .validationsupport import ValidationSupport
from .closabledialog import ClosableDialog
from .analysismoduleselector import AnalysisModuleSelector
from .activelabel import ActiveLabel
from .searchbox import SearchBox
from .caseselector import CaseSelector
from .caselist import CaseList
from .checklist import CheckList
from .stringbox import StringBox
from .listeditbox import ListEditBox
from .customdialog import CustomDialog
from .summarypanel import SummaryPanel
from .pathchooser import PathChooser

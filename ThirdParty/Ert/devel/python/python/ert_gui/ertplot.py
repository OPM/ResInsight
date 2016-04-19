
import sys
import os
from PyQt4.QtCore import Qt
from PyQt4.QtGui import QApplication, QSplashScreen, QFileDialog
import time
from ert.enkf import EnKFMain
from ert.util import Version
from ert_gui.ert_splash import ErtSplash
from ert_gui.models import ErtConnector
from ert_gui.tools.plot.plot_window import PlotWindow

from ert_gui.widgets import util

import ert_gui.widgets.util

if os.getenv("ERT_SHARE_PATH"):
    ert_share_path = os.getenv("ERT_SHARE_PATH")
else:
    # If the ERT_SHARE_PATH variable is not set we try to use the
    # source location relative to the location of the current file;
    # assuming we are in the source directory. Will not work if we are
    # in an arbitrary build directory.
    ert_share_path = os.path.realpath( os.path.join( os.path.dirname( os.path.abspath( __file__)) , "../../../share"))
    
ert_gui.widgets.util.img_prefix = ert_share_path + "/gui/img/"

from ert_gui.widgets.util import resourceImage


def main(argv):

    app = QApplication(argv) #Early so that QT is initialized before other imports
    app.setWindowIcon(util.resourceIcon("application/window_icon_cutout"))

    if len(argv) == 1:
        sys.stderr.write("Missing configuration file")
        sys.exit(1)

    config_file = argv[1]
    strict = True
        
    if not os.path.exists(config_file):
        print("Can not run without a configuration file.")
        sys.exit(1)

    if os.path.isdir(config_file):
        print("The specified configuration file is a directory!")
        sys.exit(1)


    splash = ErtSplash()
    splash.version = "Version %s" % Version.getVersion()
    splash.timestamp = Version.getBuildTime()

    splash.show()
    splash.repaint()

    now = time.time()


    ert = EnKFMain(config_file, strict=strict, verbose=False)
    ErtConnector.setErt(ert)

    window = PlotWindow(ert, None)

    sleep_time = 2 - (time.time() - now)

    if sleep_time > 0:
        time.sleep(sleep_time)

    window.show()
    splash.finish(window)
    window.activateWindow()
    window.raise_()
    finished_code = app.exec_()

    ert.free()

    sys.exit(finished_code)


if __name__ == "__main__":
    main(sys.argv)








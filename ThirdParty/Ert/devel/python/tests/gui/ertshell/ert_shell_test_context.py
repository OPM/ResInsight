from StringIO import StringIO
import os
import sys
from ert.test import TestAreaContext
from ert_gui.shell import ErtShell

class ShellCapturing(list):

    def __init__(self, shell):
        self.shell = shell

    def __enter__(self):
        self._stdout = sys.stdout
        self._shell_stdout = self.shell.stdout
        sys.stdout = self.shell.stdout = self._stringio = StringIO()
        return self

    def __exit__(self, *args):
        self.extend(self._stringio.getvalue().splitlines())
        sys.stdout = self._stdout
        self.shell.stdout = self._shell_stdout



class ErtShellTestContext(object):

    def __init__(self, test_name, config_file, load_config=True, prefix=None, store_area=False):
        self.config_file = config_file
        self.load_config = load_config
        self.test_area_context = TestAreaContext(test_name, prefix=prefix, store_area=store_area)


    def __enter__(self):
        """ :rtype: ErtShell """
        test_area = self.test_area_context.__enter__()

        if os.path.exists(self.config_file):
            test_area.copy_parent_content(self.config_file)
        elif self.config_file is not None:
            raise IOError("The config file: '%s' does not exist!" % self.config_file)

        self.shell = ErtShell(forget_history=True)

        config_file = os.path.basename(self.config_file)

        if self.load_config:
            self.shell.onecmd("load_config %s" % config_file)

        return self.shell


    def __exit__(self, exc_type, exc_val, exc_tb):
        self.shell.do_exit("")
        self.shell._cleanup()
        self.test_area_context.__exit__(exc_type, exc_val, exc_tb)
        return False
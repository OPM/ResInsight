import os
import stat
import sys

import ert
from ert.test.extended_testcase import ExtendedTestCase
from ert.test.test_area import TestAreaContext

from cwrap import Prototype

class _TestSpawnPrototype(Prototype):
    lib = ert.load('libert_util')

    def __init__(self, prototype, bind=True):
        super(_TestSpawnPrototype, self).__init__(_TestSpawnPrototype.lib, prototype, bind=bind)


class SpawnTest(ExtendedTestCase):
    _spawn = _TestSpawnPrototype("int util_spawn_blocking(char*, int, void*, char*, char*)", bind = False)
    def createScript(self, name, stdout_string , stderr_string):
        with open(name, "w") as f:
            f.write("#!/usr/bin/env python\n")
            f.write("import sys\n")
            f.write("sys.stdout.write('%s')\n" % stdout_string)
            f.write("sys.stdout.flush()\n")
            f.write("sys.stderr.write('%s')\n" % stderr_string)
            f.write("sys.stderr.flush()\n")

        mode = os.stat(name).st_mode
        mode |= stat.S_IXUSR | stat.S_IXGRP
        os.chmod(name, stat.S_IMODE(mode))
        


    def test_spawn_redirect(self):
        with TestAreaContext("spawn_test1", store_area=True) as test_area:
            stdout_string = "stdout_redirect"
            stderr_string = "stderr_redirect"
            self.createScript("print.py",stdout_string, stderr_string)
            status = self._spawn("print.py",0, None, "print.out", "print.err")
            self.assertEqual(status , 0)
            
            sys.stderr.write("init stderr\n")
            sys.stdout.write("init stdout\n")
            sys.stderr.write("complete stderr\n")
            sys.stdout.write("complete stdout\n")
            
            with open("print.out", "r") as f:
                s = f.read()
                self.assertEqual(s , stdout_string)
                
            with open("print.err", "r") as f:
                s = f.read()
                self.assertEqual(s , stderr_string)

                

    def test_spawn_noredirect(self):
        with TestAreaContext("spawn_test2", store_area=True) as test_area:
            self.createScript("print.py","stdout_no_redirect", "stderr_no_redirect")
            status = self._spawn("print.py", 0, None, None , None)
            self.assertEqual(status , 0)
            

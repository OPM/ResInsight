import os
from unittest2 import skip
from ert.util import LaTeX
from ert_tests import ExtendedTestCase


class LatexTest(ExtendedTestCase):

    def setUp(self):
        self.local_path = self.createTestPath("local/util/latex")
        self.statoil_path = self.createTestPath("Statoil/util/latex")


    def test_simple_compile(self):
        lx = LaTeX("%s/test_OK.tex" % self.local_path)
        self.assertTrue(lx.compile())

        lx = LaTeX("%s/test_error.tex" % self.local_path)
        self.assertFalse(lx.compile())

    @skip("Unknown errors!")
    def test_cleanup( self ):
        lx = LaTeX("%s/report_OK.tex" % self.statoil_path, in_place=True)
        self.assertTrue(lx.in_place)
        self.assertTrue(lx.compile())
        for ext in ["log", "aux", "nav", "out", "snm", "toc"]:
            self.assertFalse(os.path.exists("%s/report_OK.%s" % (self.statoil_path, ext)))

        lx = LaTeX("%s/report_OK.tex" % self.statoil_path, in_place=False)
        self.assertFalse(lx.in_place)
        run_path = lx.runpath
        self.assertTrue(lx.compile())
        self.assertFalse(os.path.exists(run_path))

        lx = LaTeX("%s/report_OK.tex" % self.statoil_path, in_place=False)
        run_path = lx.runpath
        self.assertTrue(lx.compile(cleanup=False))
        self.assertTrue(os.path.exists("%s/report_OK.log" % run_path))


    @skip("Unknown errors!")
    def test_report(self):
        lx = LaTeX("%s/report_error.tex" % self.statoil_path)
        lx.timeout = 4
        self.assertFalse(lx.compile())

        lx = LaTeX("%s/report_OK.tex" % self.statoil_path)
        self.assertTrue(lx.compile())

    @skip("Unknown errors!")
    def test_target(self):
        lx = LaTeX("%s/report_OK.tex" % self.statoil_path)
        self.assertTrue(lx.compile())
        self.assertTrue(os.path.exists(lx.target))
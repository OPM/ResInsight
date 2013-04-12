#!/usr/bin/env python
#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'latex_test.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 

import datetime
import unittest
import os.path
import ert
import ert.util.latex as latex
from   test_util import approx_equal, approx_equalv

local_path   = "test-data/local/util/latex"
statoil_path = "test-data/Statoil/util/latex"

class LaTeXTest( unittest.TestCase ):
    def setUp(self):
        pass


    def test1(self):
        lx = latex.LaTeX( "%s/test_OK.tex" % local_path )
        self.assertTrue( lx.compile( ) )

        lx = latex.LaTeX( "%s/test_error.tex" % local_path )
        self.assertFalse( lx.compile( ) )



    def test_cleanup( self ):
        lx = latex.LaTeX( "%s/report_OK.tex" % statoil_path , in_place = True )
        self.assertTrue( lx.in_place )
        self.assertTrue( lx.compile() )
        for ext in ["log" , "aux" , "nav" , "out" , "snm" , "toc"]:
            self.assertFalse( os.path.exists( "%s/report_OK.%s" % (statoil_path , ext) ))

        lx = latex.LaTeX( "%s/report_OK.tex" % statoil_path , in_place = False )
        self.assertFalse( lx.in_place )
        run_path = lx.runpath
        self.assertTrue( lx.compile() )
        self.assertFalse( os.path.exists( run_path ) )

        lx = latex.LaTeX( "%s/report_OK.tex" % statoil_path , in_place = False )
        run_path = lx.runpath
        self.assertTrue( lx.compile( cleanup = False) )
        self.assertTrue( os.path.exists( "%s/report_OK.log" % run_path))



    def test_report(self):
        lx = latex.LaTeX( "%s/report_error.tex" % statoil_path )
        lx.timeout = 4
        self.assertFalse( lx.compile() )

        lx = latex.LaTeX( "%s/report_OK.tex" % statoil_path  )
        self.assertTrue( lx.compile() )
                              

    def test_target(self):
        lx = latex.LaTeX( "%s/report_OK.tex" % statoil_path  )
        self.assertTrue( lx.compile() )
        self.assertTrue( os.path.exists( lx.target ))



def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( LaTeXTest( 'test1' ))
    suite.addTest( LaTeXTest( 'test_report' ))
    suite.addTest( LaTeXTest( 'test_cleanup' ))
    suite.addTest( LaTeXTest( 'test_target' ))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )

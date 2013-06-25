#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'import_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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


def test_import( module ):
    print "Importing: %s ..." % module , 
    __import__( module )
    print


test_import( "ert" )

test_import( "ert.cwrap" )
test_import( "ert.ecl" )
test_import( "ert.util" )
test_import( "ert.geo" )
test_import( "ert.config" )
test_import( "ert.job_queue" )
test_import( "ert.rms" )
test_import( "ert.enkf" )
test_import( "ert.sched" )
test_import( "ert.well")

test_import("ert.ecl.ecl")
test_import("ert.rms.rms")
test_import("ert.enkf.enkf")
test_import("ert.config.config")
test_import("ert.job_queue.job_queue")
test_import("ert.geo.geo")
test_import("ert.well.well")


def test_suite( argv ):
    return False

#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'import_local.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import sys

def test_import( module ):
    print "Importing: %s ..." % module , 
    try:
        __import__( module )
        print "OK"
    except:
        print "failed"
        sys.exit(1)


test_import("ert.ecl.ecl_local")


def test_suite( argv ):
    return False

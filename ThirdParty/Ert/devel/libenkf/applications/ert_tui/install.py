#!/usr/bin/python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'install.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import commands
import os
sys.path += ["../../../../ERT-Statoil/OldStuff/python/ctypes/SDP"]
import SDP

local_ert   = "ert"
svn_version = commands.getoutput( "svnversion" ) 

try:
    numeric = int( svn_version )
except:
    sys.exit("Will not install svn version:%s - must have a pure checkout")

svn_ert     = "%s_%s" % (local_ert , svn_version)
(SDP_ROOT , RH_version) = SDP.get_SDP_ROOT()
target_file = "%s/bin/ert_release/%s" % (SDP_ROOT, svn_ert)
ert_link    = "%s/bin/ert_latest_and_greatest" % SDP_ROOT

SDP.install_file( local_ert , target_file )
SDP.install_link( target_file , ert_link )




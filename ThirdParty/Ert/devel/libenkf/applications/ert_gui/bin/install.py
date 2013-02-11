#!/prog/sdpsoft/python2.4/bin/python
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
import os.path
import os
import re
from   stat import *
import shutil
sys.path += ["../../../../python/ctypes/SDP"]
import SDP

#################################################################

(SDP_ROOT , RH) = SDP.get_SDP_ROOT()
python_root = "%s/lib/python" % SDP_ROOT
lib_root    = "%s/lib/python/lib"  % SDP_ROOT

SDP.install_file("../../../../libutil/slib/libutil.so"           , "%s/libutil.so" % lib_root      , strict_exists = False)
SDP.install_file("../../../../libecl/slib/libecl.so"             , "%s/libecl.so" % lib_root       , strict_exists = False)
SDP.install_file("../../../../librms/slib/librms.so"             , "%s/librms.so" % lib_root       , strict_exists = False)
SDP.install_file("../../../../libenkf/slib/libenkf.so"           , "%s/libenkf.so" % lib_root      , strict_exists = False)
SDP.install_file("../../../../libconfig/slib/libconfig.so"       , "%s/libconfig.so" % lib_root    , strict_exists = False)
SDP.install_file("../../../../libjob_queue/slib/libjob_queue.so" , "%s/libjob_queue.so" % lib_root , strict_exists = False)
SDP.install_file("../../../../libplot/slib/libplot.so"           , "%s/libplot.so" % lib_root      , strict_exists = False)
SDP.install_file("../../../../libsched/slib/libsched.so"         , "%s/libsched.so" % lib_root     , strict_exists = False)

SDP.make_dir( "%s/gert" % python_root )
SDP.install_path( "code" , "%s/gert" % python_root  ,  root = "../" , extensions = ["py"])
SDP.install_path( "help" , "%s/gert" % python_root  ,  root = "../" )
SDP.install_path( "img"  , "%s/gert"  % python_root ,  root = "../" )
SDP.install_path( "doc"  , "%s/gert"  % python_root ,  root = "../" )

SDP.make_dir( "%s/gert/bin" % python_root )
SDP.install_file( "gert"        , "%s/bin/gert"        % SDP_ROOT)
SDP.install_file( "gdbcommands" , "%s/bin/gdbcommands" % SDP_ROOT)


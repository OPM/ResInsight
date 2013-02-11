#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'libjob_queue.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import os
import sys
import ctypes
import ert.util.libutil
import ert.cwrap.clib as clib


# Getting LSF to work properly is quite painful. The situation is a
# mix of internal dependencies comiled into the libjob_queue.so shared
# library, and external dependencies based on the LSF_xxx environment
# variables:
#
#   1. If the C-libraries have been compiled with LSF support,
#      i.e. the variable INCLUDE_LSF has been set to True in
#      local_config.py then the shared library libjob_queue.so will
#      depend on the libraries liblsf and libbat; this dependency
#      exists even if you have no intention actually using LSF.
#
#   2. To actually use LSF you need a whole list of environment
#      variables to be set: LSF_BINDIR , LSF_LIBDIR , XLDF_UIDDIR ,
#      LSF_SERVERDIR, LSF_ENVDIR - this is an LSF requirement and not
#      related to ERT or the Python bindings. The normal way to
#      achieve this is by sourcing a shell script.
#
# In the current code we try three different things:
#
#   1: If the environment variable LSF_HOME is set we set the
#      remaining LSF variables according to:
#
#           LSF_BINDIR    = $LSF_HOME/bin
#           LSF_LIBDIR    = $LSF_HOME/lib
#           XLSF_UIDDIR   = $LSF_HOME/lib/uid
#           LSF_SERVERDIR = $LSF_HOME/etc
#           LSF_ENVDIR    = $LSF_HOME/conf
#           PATH          = $PATH:$LSF_BINDIR
# 
#      Observe that none of these variables are modified if they
#      already have a value, furthermore it should be observed that
#      the use of an LSF_HOME variable is something invented with ERT,
#      and not standard LSF approach.
#
#   2: If the variable LSF_LIBDIR is set (either from 1: above, or
#      from external scope), we try to load the lsf libraries from
#      this location.
#
#   3. If we have no value for LSF_LIBDIR we just try a wild shot for
#      loading the LSF libraries.
#
#
# When we have tried to load the LSF libraries we continue on to load
# the libjob_queue.so ERT library. Then the following possibilities
# exist:
#
#   1. The libjob_queue library has been built without LSF support,
#      i.e. INCLUDE_LSF == False, in this case the loading of
#      libjob_queue.so is "guaranteed" to suceed.
#
#  2.  The libjob_queue library has been built with LSF support,
#      i.e. INCLUDE_LSF == True:
#
#      - If we succeeded in loading liblsf/libbat in the previous
#        section, loading libjob_queue should.so work out OK.
#
#      - If we failed to load libbat/liblsf libjob_queue will not
#        load, and the whole thing will go up in flames.
# 


def setenv( var , value):
    if not os.getenv( var ):
        os.environ[ var ] = value

# 1: Setting up the full LSF environment
LSF_HOME = os.getenv( "LSF_HOME")
if LSF_HOME:
    setenv( "LSF_BINDIR"  , "%s/bin" % LSF_HOME )
    setenv( "LSF_LIBDIR"  , "%s/lib" % LSF_HOME )
    setenv( "XLSF_UIDDIR" , "%s/lib/uid" % LSF_HOME )
    setenv( "LSF_SERVERDIR" , "%s/etc" % LSF_HOME)
    setenv( "LSF_ENVDIR" , "%s/conf" % LSF_HOME)   # This one might be too simple minded.


# 2: Loading the LSF libraries
LSF_LIBDIR = os.getenv("LSF_LIBDIR")
try:
    clib.load("libnsl.so" , "libnsl.so.1")
    clib.load("libnsl.so.1")
    if LSF_LIBDIR:
        clib.load("%s/liblsf.so" % LSF_LIBDIR)
        clib.load("%s/libbat.so" % LSF_LIBDIR)
    else:
        clib.load( "liblsf.so" )
        clib.load( "libbat.so" )
    HAVE_LSF = True
except:
    HAVE_LSF = False


# 3: Loading the libjob_queue library, which might (depending on the
#    value of INCLUDE_LSF used when building) depend on the LSF
#    libraries we tried to load at the previous step.
clib.ert_load("libconfig.so" )
try:
    lib  = clib.ert_load("libjob_queue.so")
except:
    if HAVE_LSF == False:
        sys.stderr.write("** Failed to load the libjob_queue library, \n")
        sys.stderr.write("** have previosuly failed to load the LSF\n")
        sys.stderr.write("** libraries liblsf & libbat - that might be\n")
        sys.stderr.write("** the reason ... ")
        if LSF_LIBDIR:
            sys.stderr.write("** LSF_LIBDIR = %s\n" % LSF_LIBDIR)
        else:
            sys.stderr.write("** LSF_LIBDIR = <NOT SET>\n")
    sys.exit("Failed to load library: libjob_queue")



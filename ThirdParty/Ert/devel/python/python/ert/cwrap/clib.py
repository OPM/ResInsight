#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'clib.py' is part of ERT - Ensemble based Reservoir Tool. 
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
"""
Convenience module for loading shared library.
"""

import ctypes
import os
import os.path

ert_lib_path = None


def __load__( lib_list , ert_prefix):
    """
    Thin wrapper around the ctypes.CDLL function for loading shared library.
    
    The shared libraries typically exist under several different
    names, with different level of version detail. Unfortunately the
    same library can exist under different names on different
    computers, to support this the load function can get several
    arguments like:
    
       load("libz.so" , "libz.so.1" , "libz.so.1.2.1.2" , "libZ-fucker.so")
    
    Will return a handle to the first successfull load, and raise
    ImportError if none of the loads succeed.
    """
    error_list = {}
    dll = None
    for lib in lib_list:
        try:
            if ert_prefix:
                ert_lib = os.path.join( ert_lib_path , lib )
                dll = ctypes.CDLL( ert_lib , ctypes.RTLD_GLOBAL )
            else:
                dll = ctypes.CDLL( lib , ctypes.RTLD_GLOBAL )
            return dll
        except Exception,exc:
            error_list[lib] = exc

    error_msg = "\nFailed to load shared library:%s\n\ndlopen() error:\n" % lib_list[0]
    for lib in error_list.keys():
        error_msg += "   %16s : %s\n" % (lib , error_list[lib])
    error_msg += "\n"

    LD_LIBRARY_PATH = os.getenv("LD_LIBRARY_PATH")
    if not LD_LIBRARY_PATH:
        LD_LIBRARY_PATH = ""

    error_msg += """
The runtime linker has searched through the default location of shared
libraries, and also the locations mentioned in your LD_LIBRARY_PATH
variable. Your current LD_LIBRARY_PATH setting is:

   LD_LIBRARY_PATH: %s

You might need to update this variable?
""" % LD_LIBRARY_PATH
    raise ImportError( error_msg )

#################################################################


def load( *lib_list ):
    """
    Will try to load shared library with normal load semantics.
    """
    return __load__(lib_list , False )


def ert_load( *lib_list ):
    """
    Iff the ert_lib_path module variable has been set it will try to
    load shared library from that path; if that fails the loader will
    try again without imposing any path restrictions.
    """
    if ert_lib_path:
        try:
            return __load__(lib_list , True )
        except ImportError:
            # Try again - ignoring the ert_lib_path setting.
            return load( lib_list )
    else:
        # The ert_lib_path variable has not been set; just try a normal load.
        return load( lib_list )

#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'clib.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify it under the
#  terms of the GNU General Public License as published by the Free Software
#  Foundation, either version 3 of the License, or (at your option) any later
#  version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
#  A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.
"""Convenience module for loading shared library.
"""

import platform
import ctypes
import os

so_extension = {"linux"  : "so",
                "linux2" : "so",
                "linux3" : "so",
                "win32"  : "dll",
                "win64"  : "dll",
                "darwin" : "dylib" }


# Passing None to the CDLL() function means to open a lib handle to
# the current runnning process, i.e. like dlopen( NULL ). We must
# special case this to avoid creating the bogus argument 'None.so'.

def lib_name(lib , path = None , so_version = ""):
    if lib is None:
        return None
    else:
        platform_key = platform.system().lower()

        if platform_key == "darwin":
            so_name = "%s%s.%s" % (lib, so_version, so_extension[ platform_key ])
        else:
            so_name = "%s.%s%s" % (lib, so_extension[ platform_key ], so_version)

        if path:
            return os.path.join( path , so_name )
        else:
            return so_name




def load( lib, so_version = None, path = None):
    """Thin wrapper around the ctypes.CDLL function for loading shared
    library.

    If the path argument is non Null the function will first try to
    load with full path. If that fails it wil also try to load without
    a path component, invoking normal dlopen() semantics.
    """

    dll = None
    lib_files = [ lib_name( lib , path = path , so_version = so_version) ]
    if path:
        lib_files.append( lib_name( lib , path = None , so_version = so_version) )

    for lib_file in lib_files:
        try:
            dll = ctypes.CDLL(lib_file , ctypes.RTLD_GLOBAL)
            return dll
        except Exception as exc:
            error = exc

    error_msg = "\nFailed to load shared library:%s\n\ndlopen() error: %s\n" % (lib , error)

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
    raise ImportError(error_msg)

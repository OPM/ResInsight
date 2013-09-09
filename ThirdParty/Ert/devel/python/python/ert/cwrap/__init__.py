#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file '__init__.py' is part of ERT - Ensemble based Reservoir Tool. 
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
The cwrap package contains several small utility modules to simplify
the process of interacting with a C library:

  cenum: This module contains a function 'create_enum()' which can
     internalize enum symbols with the corresponding values in the
     calling scope. The enum symbols are not part of the shared
     library as such, and the working of create_enum() requires a
     special function for 'enum-introspection' to be available in the
     shared library.

  clib: This module contains the function load() which will load a
     shared library using the ctypes.CDLL(); the function has
     facilities for trying several different names when loading the
     library.

  cwrap: This module contains support for a Python <-> C type map. The
     whole type mapping in the ert python bindings is based on this
     module.

  cfile: This module implemenets the class CFILE which can be used to
     extract the underlying FILE pointer from a Python filehandle, to
     facilitate use of Python filehandles for functions expecting a
     FILE pointer.
"""

from .cclass import CClass
from .cenum import create_enum
from .cfile import CFILE
from .clib import load, ert_lib_path, ert_load
from .cwrap import CWrapper, CWrapperNameSpace

from .cnamespace import CNamespace
from .basecclass import BaseCClass
from .basecenum import BaseCEnum


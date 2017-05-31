#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

  clib: This module contains the function load() which will load a
     shared library using the ctypes.CDLL(); the function has
     facilities for trying several different names when loading the
     library.

  cfile: This module implemenets the class CFILE which can be used to
     extract the underlying FILE pointer from a Python filehandle, to
     facilitate use of Python filehandles for functions expecting a
     FILE pointer.
"""

__author__ = 'Jean-Paul Balabanian, Joakim Hove, and PG Drange'
__copyright__ = 'Copyright 2016, Statoil ASA'
__credits__ = __author__
__license__ = 'GPL'
__version__ = '0.0.1'
__maintainer__ = __author__
__email__ = __author__
__status__ = 'Prototype'

from .basecclass import BaseCClass
from .basecenum import BaseCEnum
from .basecvalue import BaseCValue

from .cfile import CFILE
from .clib import load, lib_name

from .metacwrap import MetaCWrap
from .prototype import REGISTERED_TYPES, Prototype, PrototypeError

__all__ = ['BaseCClass', 'BaseCEnum', 'BaseCValue', 'CFILE',
           'MetaCWrap', 'Prototype', 'load', 'lib_name']

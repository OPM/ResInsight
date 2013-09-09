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
Package with utility classes, used by other ERT classes.

The libutil library implements many utility functions and classes of
things like hash table and vector; these classes are extensively used
by the other ert libraries. The present wrapping here is to facilitate
use and interaction with various ert classes, in a pure python context
you are probably better served by using a plain python solution;
either based on built in python objects or well established third
party packages.

The modules included in the util package are:

  tvector.py: This module implements the classes IntVector,
     DoubleVector and BoolVector. This is a quite normal
     implementation of a typed growable vector; but with a special
     twist regarding default values.

  util_func.py: This module wraps a couple of stateless (i.e. there is
     no class involved) functions from the util.c file.
   
"""

import ert.cwrap.clib as clib

clib.load("libz" , "libz.so.1")

# Depending on the Fortran compiler which has been used to compile
# blas / lapack the there might be an additional dependency on libg2c:

try:
    # First try to load without libg2c
    clib.load("libblas.so" , "libblas.so.3")
    clib.load("liblapack.so")
except ImportError:
    # Then try to load with libg2c
    clib.load("libg2c.so.0")
    clib.load("libblas.so" , "libblas.so.3")
    clib.load("liblapack.so")

UTIL_LIB = clib.ert_load("libert_util.so")

from .tvector import DoubleVector, IntVector, BoolVector, TimeVector, TVector
from .stringlist import StringList
from .stat import quantile, quantile_sorted
from .matrix import Matrix
from .log import Log
from .lookup_table import LookupTable
from .buffer import Buffer
from .ctime import ctime
from .hash import Hash, StringHash, DoubleHash, IntegerHash
from .latex import LaTeX
from .substitution_list import SubstitutionList
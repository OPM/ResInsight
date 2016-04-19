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

# The libert_util library requires the libraries libz, libblas and
# liblapack. It is assumed that the library has been compiled with a
# suitable RPATH option (i.e. ERT_USE_RPATH has been set to True in
# the build process) and we 'just' load the final libert_util library
# directly. In principle it would be possible preload these libraries
# with calls like:
#
# clib.load("libz" , "libz.so.1")
# clib.load("libblas" , "libblas.so" , "libblas.so.3")
# clib.load("liblapack" , "liblapack.so")
from ert.cwrap.metacwrap import Prototype


class UtilPrototype(Prototype):
    lib = clib.ert_load("libert_util")

    def __init__(self, prototype, bind=True):
        super(UtilPrototype, self).__init__(UtilPrototype.lib, prototype, bind=bind)



from .version import Version

from .enums import RngAlgTypeEnum, RngInitModeEnum, LLSQResultEnum

from .ctime import CTime

from .vector_template import VectorTemplate, PermutationVector
from .double_vector import DoubleVector
from .int_vector import IntVector
from .bool_vector import BoolVector
from .time_vector import TimeVector
from .stringlist import StringList
from .rng import RandomNumberGenerator
from .matrix import Matrix
from .stat import quantile, quantile_sorted, polyfit
from .log import Log
from .lookup_table import LookupTable
from .buffer import Buffer
from .hash import Hash, StringHash, DoubleHash, IntegerHash
from .substitution_list import SubstitutionList
from .ui_return import UIReturn
from .thread_pool import ThreadPool
from .cthread_pool import CThreadPool, startCThreadPool
from .install_abort_signals import installAbortSignals, updateAbortSignals
from .profiler import Profiler
from .arg_pack import ArgPack
from .path_format import PathFormat

# This is only imported for the prototype side-effect; no symbols from
# this module are actually used.
import cstring

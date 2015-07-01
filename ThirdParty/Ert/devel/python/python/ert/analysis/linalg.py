#  Copyright (C) 2015  Statoil ASA, Norway.
#
#  The file 'linalg.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.cwrap import BaseCClass, CWrapper
from ert.analysis import ANALYSIS_LIB


__all__ = ["numPC"]

class Linalg(BaseCClass):
    """
    The linalg class is a purely static class which mainly serves as a
    namespace for a collection of ensemble based linear algebra
    methods.
    """
    
    @staticmethod
    def numPC(S , truncation):
        if 0 < truncation <= 1:
            return Linalg.cNamespace().get_num_PC( S , truncation )
        else:
            raise ValueError("truncation must be in the interval (0,1]")


cwrapper = CWrapper(ANALYSIS_LIB)
Linalg.cNamespace().get_num_PC = cwrapper.prototype("int enkf_linalg_num_PC( matrix , double)")

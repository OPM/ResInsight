#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'stat.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from collections import Sequence

from cwrap import PrototypeError
from ecl.util import LLSQResultEnum, UtilPrototype, Matrix


quantile = UtilPrototype("double statistics_empirical_quantile(double_vector, double)")
"""@type: (ecl.util.DoubleVector, float)->float"""

quantile_sorted = UtilPrototype("double statistics_empirical_quantile(double_vector, double)")
"""@type: (ecl.util.DoubleVector, float)->float"""

try:
    _polyfit = UtilPrototype("llsq_result_enum matrix_stat_polyfit(matrix, matrix, matrix, matrix)")
except PrototypeError:
    _polyfit = None

def polyfit(n, x, y, s=None):
    """
    @type n: int
    @type x: Matrix or Sequence
    @type y: Matrix or Sequence
    @type s: Matrix or Sequence or None
    @return: tuple
    """
    if _polyfit is None:
        raise NotImplementedError("Sorry - your ert distribution has been built without lapack support")

    if isinstance(x, Matrix):
        xm = x
    else:
        xm = Matrix(len(x), 1)
        for i in range(len(x)):
            xm[i, 0] = x[i]

    if isinstance(y, Matrix):
        ym = y
    else:
        ym = Matrix(len(y), 1)
        for i in range(len(y)):
            ym[i, 0] = y[i]

    if s:
        if isinstance(s, Matrix):
            sm = s
        else:
            sm = Matrix(len(s), 1)
            for i in range(len(s)):
                sm[i, 0] = s[i]
    else:
        sm = s

    beta = Matrix(n, 1)
    res = _polyfit(beta, xm, ym, sm)

    if not res == LLSQResultEnum.LLSQ_SUCCESS:
        raise Exception("Linear Least Squares Estimator failed?")

    l = []
    for i in range(n):
        l.append(beta[i, 0])

    return tuple(l)

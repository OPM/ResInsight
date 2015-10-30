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


from ert.util import UTIL_LIB
from ert.util import Matrix
from ert.util import LLSQResultEnum
from ert.cwrap import CWrapper, CWrapperNameSpace, CWrapError


def quantile( data, q ):
    return cfunc.quantile(data, q)


def quantile_sorted( data, q ):
    return cfunc.quantile_sorted(data, q)


def polyfit(n,x,y,s = None):
    if cfunc.polyfit is None:
        raise NotImplementedError("Sorry - your ert distribution has been built without lapack support")
    
    if isinstance(x,Matrix):
        xm = x
    else:
        xm = Matrix( len(x) , 1 )
        for i in range(len(x)):
            xm[i,0] = x[i]

    if isinstance(y,Matrix):
        ym = y
    else:
        ym = Matrix( len(y) , 1 )
        for i in range(len(y)):
            ym[i,0] = y[i]

    if s:
        if isinstance(s , Matrix):
            sm = s
        else:
            sm = Matrix( len(s) , 1 )
            for i in range(len(s)):
                sm[i,0] = s[i]
    else:
        sm = s

    
    beta = Matrix( n , 1 )
    res = cfunc.polyfit( beta , xm , ym , sm)
    
    if not res == LLSQResultEnum.LLSQ_SUCCESS:
        raise Exception("Linear Least Squares Estimator failed?")

    l = []
    for i in range(n):
        l.append( beta[i,0] )

    return tuple(l)



            




cwrapper = CWrapper(UTIL_LIB)
cfunc = CWrapperNameSpace("stat")

cfunc.quantile = cwrapper.prototype("double statistics_empirical_quantile( double_vector , double )")
cfunc.quantile_sorted = cwrapper.prototype("double statistics_empirical_quantile( double_vector , double )")
try:
    cfunc.polyfit = cwrapper.prototype("llsq_result_enum matrix_stat_polyfit(matrix , matrix , matrix , matrix)")
except CWrapError:
    cfunc.polyfit = None


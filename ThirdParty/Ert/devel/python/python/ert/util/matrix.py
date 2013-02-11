#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'matrix.py' is part of ERT - Ensemble based Reservoir Tool. 
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


# The Matrix class implemented here wraps the C matrix implementation
# in matrix.c from the libutil library. The C matrix implementation
# has the very limited ambition of just barely satisfying the matrix
# needs of the EnKF algorithm, i.e. for general linear algebra
# applications you will probably be better served by a more complete
# matrix library. This applies even more so to this Python
# implementation; it is only here facilitate use of C libraries which
# expect a matrix instance as input (i.e. the LARS estimator). For
# general linear algebra in Python the numpy library is a natural
# choice.


import sys
import ctypes
import libutil
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass

class Matrix(CClass):

    def __init__(self , rows , columns):
        c_ptr = cfunc.matrix_alloc( rows , columns )
        self.init_cobj( c_ptr , cfunc.free )
            
    def __getitem__(self, index_tuple ):
        print index_tuple
        (i,j) = index_tuple
        return cfunc.iget( self , i,j)

    def __setitem__(self, index_tuple , value):
        (i,j) = index_tuple
        return cfunc.iset( self , i,j , value)


#################################################################

CWrapper.registerType( "matrix" , Matrix )
cwrapper = CWrapper( libutil.lib )
cfunc    = CWrapperNameSpace("matrix")

cfunc.matrix_alloc = cwrapper.prototype("c_void_p   matrix_alloc( int , int )")
cfunc.free         = cwrapper.prototype("void       matrix_free( matrix )")
cfunc.iget         = cwrapper.prototype("double     matrix_iget( matrix , int , int )")
cfunc.iset         = cwrapper.prototype("void       matrix_iset( matrix , int , int , double)")
        
    

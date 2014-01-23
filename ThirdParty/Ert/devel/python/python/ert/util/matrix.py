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


from ert.util import UTIL_LIB
from ert.cwrap import CWrapper, BaseCClass


class Matrix(BaseCClass):
    def __init__(self, rows, columns):
        self.__rows = rows
        self.__columns = columns
        c_ptr = Matrix.cNamespace().matrix_alloc(rows, columns)
        super(Matrix, self).__init__(c_ptr)

    def __getitem__(self, index_tuple):
        if not 0 <= index_tuple[0] < self.__rows:
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.__rows))

        if not 0 <= index_tuple[1] < self.__columns:
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.__columns))

        return Matrix.cNamespace().iget(self, index_tuple[0], index_tuple[1])

    def __setitem__(self, index_tuple, value):
        if not 0 <= index_tuple[0] < self.__rows:
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.__rows))

        if not 0 <= index_tuple[1] < self.__columns:
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.__columns))

        return Matrix.cNamespace().iset(self, index_tuple[0], index_tuple[1], value)

    def free(self):
        Matrix.cNamespace().free(self)


#################################################################

cwrapper = CWrapper(UTIL_LIB)
CWrapper.registerType("matrix", Matrix)
CWrapper.registerType("matrix_obj", Matrix.createPythonObject)
CWrapper.registerType("matrix_ref", Matrix.createCReference)

Matrix.cNamespace().matrix_alloc = cwrapper.prototype("c_void_p matrix_alloc(int, int )")
Matrix.cNamespace().free = cwrapper.prototype("void   matrix_free(matrix)")
Matrix.cNamespace().iget = cwrapper.prototype("double matrix_iget( matrix , int , int )")
Matrix.cNamespace().iset = cwrapper.prototype("void   matrix_iset( matrix , int , int , double)")
        
    

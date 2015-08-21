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
    def __init__(self, rows, columns , value = 0):
        c_ptr = Matrix.cNamespace().matrix_alloc(rows, columns)
        super(Matrix, self).__init__(c_ptr)
        self.setAll(value)


    def __str__(self):
        s = ""
        for i in range(self.rows()):
            s += "["
            for j in range(self.columns()):
                d = Matrix.cNamespace().iget(self, i,j)
                s += "%6.3g " % d
            s += "]\n"
        return s


    def __getitem__(self, index_tuple):
        if not 0 <= index_tuple[0] < self.rows():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.rows()))

        if not 0 <= index_tuple[1] < self.columns():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.columns()))

        return Matrix.cNamespace().iget(self, index_tuple[0], index_tuple[1])


    def __setitem__(self, index_tuple, value):
        if not 0 <= index_tuple[0] < self.rows():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.rows()))

        if not 0 <= index_tuple[1] < self.columns():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.columns()))

        return Matrix.cNamespace().iset(self, index_tuple[0], index_tuple[1], value)


    def dims(self):
        return (Matrix.cNamespace().rows(self) , Matrix.cNamespace().columns(self))


    def rows(self):
        """ @rtype: int """
        return Matrix.cNamespace().rows(self)


    def columns(self):
        """ @rtype: int """
        return Matrix.cNamespace().columns(self)


    def __eq__(self, other):
        assert isinstance(other, Matrix)
        return Matrix.cNamespace().equal(self, other)

    def scaleColumn(self, column , factor):
        if not 0 <= column < self.columns():
            raise IndexError("Expected column: [0,%d) got:%d" % (self.columns() , column))
        Matrix.cNamespace().scale_column(self , column , factor)

    def scaleRow(self, row , factor):
        if not 0 <= row < self.rows():
            raise IndexError("Expected row: [0,%d) got:%d" % (self.rows() , row))
        Matrix.cNamespace().scale_row(self , row ,  factor)
        

    def setAll(self , value):
        Matrix.cNamespace().set_all(self, value)

    def copyColumn(self , target_column , src_column):
        columns = self.columns()
        if not 0 <= src_column < columns:
            raise ValueError("src column:%d invalid" % src_column)

        if not 0 <= target_column < columns:
            raise ValueError("target column:%d invalid" % target_column)

        if src_column != target_column:
            # The underlying C function accepts column copy between matrices.
            Matrix.cNamespace().copy_column(self, self , target_column , src_column)



    def prettyPrint(self, name, fmt="%6.3g"):
        Matrix.cNamespace().pretty_print(self, name, fmt)


    def randomInit(self , rng):
        Matrix.cNamespace().random_init(self, rng)


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
Matrix.cNamespace().set_all = cwrapper.prototype("void   matrix_scalar_set( matrix , double)")
Matrix.cNamespace().scale_column = cwrapper.prototype("void matrix_scale_column(matrix , int , double)")
Matrix.cNamespace().scale_row    = cwrapper.prototype("void matrix_scale_row(matrix , int , double)")
Matrix.cNamespace().copy_column = cwrapper.prototype("void matrix_copy_column(matrix , matrix , int , int)")

Matrix.cNamespace().rows = cwrapper.prototype("int matrix_get_rows(matrix)")
Matrix.cNamespace().columns = cwrapper.prototype("int matrix_get_columns(matrix)")
Matrix.cNamespace().equal = cwrapper.prototype("bool matrix_equal(matrix, matrix)")

Matrix.cNamespace().pretty_print = cwrapper.prototype("void matrix_pretty_print(matrix, char*, char*)")
Matrix.cNamespace().random_init = cwrapper.prototype("void matrix_random_init(matrix, rng)")

    

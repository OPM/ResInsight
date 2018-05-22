
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


from cwrap import BaseCClass,CFILE
from ecl import EclPrototype


class Matrix(BaseCClass):
    _matrix_alloc      = EclPrototype("void*  matrix_alloc(int, int )" , bind = False)
    _matrix_alloc_identity = EclPrototype("matrix_obj  matrix_alloc_identity( int )" , bind = False)
    _alloc_transpose   = EclPrototype("matrix_obj  matrix_alloc_transpose(matrix)")
    _inplace_transpose = EclPrototype("void        matrix_inplace_transpose(matrix)")
    _copy              = EclPrototype("matrix_obj  matrix_alloc_copy(matrix)" )
    _sub_copy          = EclPrototype("matrix_obj  matrix_alloc_sub_copy(matrix, int , int , int , int)" )
    _free              = EclPrototype("void   matrix_free(matrix)")
    _iget              = EclPrototype("double matrix_iget( matrix , int , int )")
    _iset              = EclPrototype("void   matrix_iset( matrix , int , int , double)")
    _set_all           = EclPrototype("void   matrix_scalar_set( matrix , double)")
    _scale_column      = EclPrototype("void matrix_scale_column(matrix , int , double)")
    _scale_row         = EclPrototype("void matrix_scale_row(matrix , int , double)")
    _copy_column       = EclPrototype("void matrix_copy_column(matrix , matrix , int , int)" , bind = False)
    _rows              = EclPrototype("int matrix_get_rows(matrix)")
    _columns           = EclPrototype("int matrix_get_columns(matrix)")
    _equal             = EclPrototype("bool matrix_equal(matrix, matrix)")
    _pretty_print      = EclPrototype("void matrix_pretty_print(matrix, char*, char*)")
    _fprint            = EclPrototype("void matrix_fprintf(matrix, char*, FILE)")
    _random_init       = EclPrototype("void matrix_random_init(matrix, rng)")
    _dump_csv          = EclPrototype("void matrix_dump_csv(matrix, char*)")

    # Requires BLAS. If the library does not have the
    # matrix_alloc_matmul() function the prototype will have _func =
    # None, and NotImplementedError( ) will be raised int the
    # __call__() method if we try to use this function.
    try:
        _alloc_matmul = EclPrototype("matrix_obj  matrix_alloc_matmul(matrix, matrix)" , bind = False)
    except AttributeError:
        _alloc_matmul = None

    # Requires BLAS!
    @classmethod
    def matmul(cls, m1,m2):
        """
        Will return a new matrix which is matrix product of m1 and m2.
        """
        if m1.columns( ) == m2.rows( ):
            return cls._alloc_matmul( m1, m2)
        else:
            raise ValueError("Matrix size mismatch")


    def __init__(self, rows, columns, value=0):
        c_ptr = self._matrix_alloc(rows, columns)
        super(Matrix, self).__init__(c_ptr)
        self.setAll(value)

    def copy(self):
        return self._copy( )

    @classmethod
    def identity(cls, dim):
        """Returns a dim x dim identity matrix."""
        if dim < 1:
            raise ValueError('Identity matrix must have positive size, %d not allowed.' % dim)
        return cls._matrix_alloc_identity(dim)

    def subCopy(self, row_offset, column_offset, rows, columns):
        if row_offset < 0 or row_offset >= self.rows():
            raise ValueError("Invalid row offset")

        if column_offset < 0 or column_offset >= self.columns():
            raise ValueError("Invalid column offset")

        if row_offset + rows > self.rows():
            raise ValueError("Invalid rows")

        if column_offset + columns > self.columns():
            raise ValueError("Invalid columns")

        return self._sub_copy( row_offset , column_offset , rows , columns)


    def __str__(self):
        s = ""
        for i in range(self.rows()):
            s += "["
            for j in range(self.columns()):
                d = self._iget(i, j)
                s += "%6.3g " % d
            s += "]\n"
        return s

    def __getitem__(self, index_tuple):
        if not 0 <= index_tuple[0] < self.rows():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.rows()))

        if not 0 <= index_tuple[1] < self.columns():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.columns()))

        return self._iget(index_tuple[0], index_tuple[1])

    def __setitem__(self, index_tuple, value):
        if not 0 <= index_tuple[0] < self.rows():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[0], self.rows()))

        if not 0 <= index_tuple[1] < self.columns():
            raise IndexError("Expected 0 <= %d < %d" % (index_tuple[1], self.columns()))

        return self._iset(index_tuple[0], index_tuple[1], value)

    def dims(self):
        return self._rows(), self._columns()

    def rows(self):
        """ @rtype: int """
        return self._rows()

    def transpose(self , inplace = False):
        """
        Will transpose the matrix. By default a transposed copy is returned.
        """
        if inplace:
            self._inplace_transpose( )
            return self
        else:
            return self._alloc_transpose( )


    def columns(self):
        """ @rtype: int """
        return self._columns()

    def __eq__(self, other):
        assert isinstance(other, Matrix)
        return self._equal(other)

    def scaleColumn(self, column, factor):
        if not 0 <= column < self.columns():
            raise IndexError("Expected column: [0,%d) got:%d" % (self.columns(), column))
        self._scale_column(column, factor)

    def scaleRow(self, row, factor):
        if not 0 <= row < self.rows():
            raise IndexError("Expected row: [0,%d) got:%d" % (self.rows(), row))
        self._scale_row(row, factor)

    def setAll(self, value):
        self._set_all(value)

    def copyColumn(self, target_column, src_column):
        columns = self.columns()
        if not 0 <= src_column < columns:
            raise ValueError("src column:%d invalid" % src_column)

        if not 0 <= target_column < columns:
            raise ValueError("target column:%d invalid" % target_column)

        if src_column != target_column:
            # The underlying C function accepts column copy between matrices.
            Matrix._copy_column(self, self, target_column, src_column)


    def dumpCSV(self , filename):
        self._dump_csv( filename )


    def prettyPrint(self, name, fmt="%6.3g"):
        self._pretty_print(name, fmt)

    def fprint(self , fileH , fmt = "%g "):
        """Will print ASCII representation of matrix.

        The fileH argument should point to an open Python
        filehandle. If you supply a fmt string it is important that it
        contains a separator, otherwise you might risk that elements
        overlap in the output. For the matrix:

                  [0 1 2]
              m = [3 4 5]
                  [6 7 8]

        The code:

        with open("matrix.txt" , "w") as f:
           m.fprintf( f )

         The file 'matrix.txt' will look like:

         0 1 2
         3 4 5
         6 7 8

        """
        self._fprint( fmt , CFILE( fileH))


    def randomInit(self, rng):
        self._random_init(rng)

    def free(self):
        self._free()




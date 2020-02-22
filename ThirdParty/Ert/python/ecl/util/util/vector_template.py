#  Copyright (C) 2014  Equinor ASA, Norway.
#
#  The file 'vector_template.py' is part of ERT - Ensemble based Reservoir Tool.
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
Typed vectors IntVector, DoubleVector and BoolVector.

This module implements a quite simple typed vector which will grow
transparently as needed. The vector is created with a default value,
which will be used for not explicitly set indices.

   vec = IntVector( default_value = 66 )
   vec[0] = 10
   vec[2] = 10

After the 'vec[2] = 10' statement the vector has grown to contain
three elements. The element vec[1] has not been explicitly assigned by
the user, in that case the implementation has 'filled the hole' with
the default value (i.e. 66 in this case). So the statement

   print(vec[1])

will give '66'. The main part of the implementation is in terms of an
"abstract base class" TVector. The TVector class should be not
instantiated directly, instead the child classes IntVector,
DoubleVector or BoolVector should be used.

The C-level has implementations for several fundamental types like
float and size_t not currently implemented in the Python version.
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import  sys

from cwrap import CFILE, BaseCClass



class VectorTemplate(BaseCClass):

    def strided_copy(self, slice_range):
        """
        Will create a new copy according to @slice.

        Mainly a support function to support slice based lookup like

           v = IntVector()
           v[0] = 1
           v[1] = 1
           v[100] = 100
           ...
           c = v[0:100:10]

        Now 'c' will be a Intvector() instance containing every tenth
        element from 'v'.
        """
        (start, stop, step) = slice_range.indices(len(self))
        if stop > start:
            return self._strided_copy(start, stop, step)
        else:
            return None

    def __bool__(self):
        """
        Will evaluate to False for empty vector.
        """
        if len(self) == 0:
            return False
        else:
            return True

    def __nonzero__(self):
        return self.__bool__( )


    def __eq__(self, other):
        return self._equal(other)


    def __ne__(self,other):
        return not self.__eq__(other)

    def first_eq(self, other, offset = 0):
        index = self._first_eq(other, offset)
        if index <= -2:
            raise ValueError("Invalid offset")

        return index


    def first_neq(self, other, offset = 0):
        index = self._first_neq(other, offset)
        if index <= -2:
            raise ValueError("Invalid offset")

        return index


    def copy(self):
        """
        Create a new copy of the current vector.
        """
        new = self._alloc_copy( )
        return new

    def __irshift__(self,shift):
        if shift < 0:
            raise ValueError("The shift must be positive")
        self._rshift(shift)
        return self

    def __ilshift__(self,shift):
        if shift < 0:
            raise ValueError("The shift must be positive")
        if shift > len(self):
            raise ValueError("The shift is too large %d > %d" % (shift, len(self)))
        self._lshift( shift)
        return self


    def __rshift__(self,shift):
        copy = self.copy()
        copy >>= shift
        return copy


    def __lshift__(self,shift):
        copy = self.copy()
        copy <<= shift
        return copy

    def __deepcopy__(self, memo):
        new = self.copy()
        return new

    def __init__(self, default_value=0, initial_size=0):
        """
        Creates a new TVector instance.
        """
        c_pointer = self._alloc(initial_size, default_value)
        super(VectorTemplate, self).__init__(c_pointer)
        self.element_size = self._element_size()

    def __contains__(self , value):
        return self._contains(  value)


    def pop(self):
        if len(self) > 0:
            return self._pop()
        else:
            raise ValueError("Trying to pop from empty vector")


    def str_data(self, width, index1, index2, fmt):
        """
        Helper function for str() method.
        """
        data = []
        s = ""
        for index in range(index1, index2):
            data.append(self[index])
        for index in range(len(data)):
            s += fmt % data[index]
            if index % width == (width - 1):
                s += "\n"
        return s


    # The str() method is a verbatim copy of the implementation in
    # ecl_kw.py.
    def str(self, width=5, max_lines=10, fmt=None):
        """
        Return string representation of vector for pretty printing.

        The function will return a string consisting of a header, and
        then a chunk of data. The data will be formatted in @width
        columns, and a maximum of @max_lines lines. If @max_lines is
        not sufficient the first elements in the kewyord are
        represented, a .... continuation line and then the last part
        of the vector. If @max_lines is None all of the vector will be
        printed, irrespectiv of how long it is.

        If a value is given for @fmt that is used as format string for
        each element, otherwise a type-specific default format is
        used. If given the @fmt string should contain spacing between
        the elements. The implementation of the builtin method
        __str__() is based on this method.
        """

        s = ""
        lines = len(self) // width
        if not fmt:
            fmt = self.default_format + " "

        if max_lines is None or lines <= max_lines:
            s += self.str_data(width, 0, len(self), fmt)
        else:
            s1 = width * max_lines // 2
            s += self.str_data(width, 0, s1, fmt)
            s += "   ....   \n"
            s += self.str_data(width, len(self) - s1, len(self), fmt)

        return s

    def __str__(self):
        """
        Returns string representantion of vector.
        """
        return self.str(max_lines=10, width=5)


    def __getitem__(self, index):
        """
        Implements read [] operator - @index can be slice instance.
        """
        if isinstance(index, int):
            length = len(self)
            idx = index
            if idx < 0:
                idx += length

            if 0 <= idx < length:
                return self._iget(idx)
            else:
                raise IndexError('Index must be in range %d <= %d < %d.' % (0, index, length))
        elif isinstance(index, slice):
            return self.strided_copy(index)
        else:
            raise TypeError("Index should be integer or slice type.")

    def __setitem__(self, index, value):
        """
        Implements write [] operator - @index must be integer or slice.
        """
        ls = len(self)
        if isinstance(index, int):
            idx = index
            if idx < 0:
                idx += ls
            self._iset(idx, value)
        elif isinstance( index, slice ):
            for i in range(*index.indices(ls)):
                self[i] = value
        else:
            raise TypeError("Index should be integer type")

    ##################################################################
    # Mathematical operations:

    def __IADD(self, delta, add):
        """
        Low-level function implementing inplace add.

        The __IADD__ function implements the operation:

           v += a

        The variable which is added, i.e. @delta, can either be of the
        same type as the current instance, or a numerical scalar. The
        __IADD__ method implements both add and subtract, based on the
        boolean flag @add.

        The __IADD__ method should not be called directly; but rather
        via the __iadd__, __add__ and __radd__ methods which implement
        the various addition operation, and the corresponding
        subtraction operations: __isub__, __sub__ and __rsub__.
        """
        if type(self) == type(delta):
            if len(self) == len(delta):
                # This is vector + vector operation.
                if not add:
                    delta *= -1
                self._inplace_add(delta)
            else:
                raise ValueError("Incompatible sizes for add self:%d  other:%d" % (len(self), len(delta)))
        else:
            if isinstance(delta, int) or isinstance(delta, float):
                if not add:
                    delta *= -1
                self._shift(delta)
            else:
                raise TypeError("delta has wrong type:%s " % type(delta))

        return self


    def __iadd__(self, delta):
        """
        Implements inplace add. @delta can be vector or scalar.
        """
        return self.__IADD(delta, True)


    def __isub__(self, delta):
        """
        Implements inplace subtract. @delta can be vector or scalar.
        """
        return self.__IADD(delta, False)


    def __radd__(self, delta):
        return self.__add__(delta)


    def __add__(self, delta):
        """
        Implements add operation - creating a new copy.

           b = DoubleVector()
           c = DoubleVector()  // Or alternatively scalar
           ....
           a = b + c
        """
        copy = self._alloc_copy( )
        copy += delta
        return copy

    def __sub__(self, delta):
        """
        Implements subtraction - creating new copy.
        """
        copy = self._alloc_copy( )
        copy -= delta
        return copy


    def __rsub__(self, delta):
        return self.__sub__(delta) * -1


    def __imul__(self, factor):
        """
        Low-level function implementing inplace multiplication.

        The __IMUL__ function implements the operation:

           v *= a

        The variable which is multiplied in, i.e. @factor, can either
        be of the same type as the current instance, or a numerical
        scalar. The __IMUL__ method should not be called directly, but
        rather via the __mul__, __imul__ and __rmul__ functions.
        """

        if type(self) == type(factor):
            # This is vector * vector operation.
            if len(self) == len(factor):
                self._inplace_mul(factor)
            else:
                raise ValueError("Incompatible sizes for mul self:%d  other:%d" % (len(self), len(factor)))
        else:
            if isinstance(factor, int) or isinstance(factor, float):
                self._scale(factor)
            else:
                raise TypeError("factor has wrong type:%s " % type(factor))

        return self


    def __mul__(self, factor):
        copy = self._alloc_copy( )
        copy *= factor
        return copy

    def __rmul__(self, factor):
        return self.__mul__(factor)

    def __div__(self, divisor):
        if isinstance(divisor, int) or isinstance(divisor, float):
            copy = self._alloc_copy()
            copy._div(divisor)
            return copy
        else:
            raise TypeError("Divisor has wrong type:%s" % type(divisor))

    def __truediv__(self, divisor):
        return self.__div__(divisor)

    def __idiv__(self, divisor):
        return self.__div__(divisor)

    def __itruediv__(self, divisor):
        return self.__div__(divisor)

    # End mathematical operations
    #################################################################

    # Essentally implements a = b
    def assign(self, value):
        """
        Implements assignment of type a = b.

        The @value parameter can either be a vector instance, in which
        case the content of @value is copied into the current
        instance, or a scalar in which case all the elements of the
        vector are set to this value:

           v1 = IntVector()
           v2 = IntVector()

           v1[10] = 77
           v2.assign( v1 )      # Now v2 contains identicall content to v1
           ....
           v1.assign( 66 )       # Now v1 is a vector of 11 elements - all equal to 66

        """
        if type(self) == type(value):
            # This is a copy operation
            self._memcpy(value)
        else:
            if isinstance(value, int) or isinstance(value, float):
                self._assign(value)
            else:
                raise TypeError("Value has wrong type")

    def __len__(self):
        """
        The number of elements in the vector.
        """
        return self._size( )


    def printf(self, fmt=None, name=None, stream=sys.stdout):
        """
        See also the str() method which returns string representantion
        of the vector.
        """
        cfile = CFILE(stream)
        if not fmt:
            fmt = self.default_format
        self._fprintf(cfile, name, fmt)


    def max(self):
        if len(self) > 0:
            return self._get_max()
        else:
            raise IndexError("The vector is empty!")

    def min(self):
        if len(self) > 0:
            return self._get_min()
        else:
            raise IndexError("The vector is empty!")


    def minIndex(self, reverse=False):
        """
        @type reverse: bool
        @rtype: int
        """
        if len(self) > 0:
            return self._get_min_index(reverse)
        else:
            raise IndexError("The vector is empty!")

    def maxIndex(self, reverse=False):
        """
        @type reverse: bool
        @rtype: int
        """
        if len(self) > 0:
            return self._get_max_index(reverse)
        else:
            raise IndexError("The vector is empty!")

    def append(self, value):
        self._append(value)

    def deleteBlock(self, index, block_size):
        """
        Remove a block of size @block_size starting at @index.

        After the removal data will be left shifted.
        """
        self._idel_block(index, block_size)

    def sort(self, reverse=False):
        """
        Sort the vector inplace in increasing numerical order or decreasing order if reverse is True.
        @type reverse: bool
        """
        if not reverse:
            self._sort()
        else:
            self._rsort()

    def clear(self):
        self._reset()

    def safeGetByIndex(self, index):
        return self._safe_iget(index)

    def setReadOnly(self, read_only):
        self._set_read_only(read_only)

    def getReadOnly(self):
        return self._get_read_only()

    def setDefault(self, value):
        self._set_default(value)

    def getDefault(self):
        return self._get_default()


    def free(self):
        self._free()

    def __repr__(self):
        return self._create_repr('size = %d' % len(self))

    def permute(self, permutation_vector):
        """
        Reorders this vector based on the indexes in permutation_vector.
        @type permutation_vector: PermutationVector
        """

        self._permute( permutation_vector)

    def permutationSort(self, reverse=False):
        """
        Returns the permutation vector for sorting of this vector. Vector is not sorted.
         @type reverse: bool
         @@rtype: PermutationVector
        """
        if len(self) > 0:
            if not reverse:
                return self._sort_perm()
            else:
                return self._rsort_perm()

        return None


    def asList(self):
        l = [0] * len(self)
        for (index,value) in enumerate(self):
            l[index] = value

        return l

    def selectUnique(self):
        self._select_unique()


    def elementSum(self):
        return self._element_sum( )


    def getDataPtr(self):
        "Low level function which returns a pointer to underlying storage"
        # Observe that the get_data_ptr() function is not implemented
        # for the TimeVector class.
        return self._get_data_ptr()

    def countEqual(self , value):
        return self._count_equal(  value )


    def initRange(self , min_value , max_value , delta):
        """
        Will fill the vector with the values from min_value to
        max_value in steps of delta. The upper limit is guaranteed to
        be inclusive, even if it is not commensurable with the delta.
        """
        if delta == 0:
            raise ValueError("Invalid range")
        else:
            self._init_range(  min_value , max_value , delta )


    @classmethod
    def create_linear(cls, start_value, end_value, num_values):
        vector = cls()
        if not vector._init_linear(start_value, end_value, num_values):
            raise ValueError("init_linear arguments invalid")

        return vector


    @classmethod
    def createRange(cls , min_value , max_value , delta):
        """
        Will create new vector and initialize a range.
        """
        vector = cls( )
        vector.initRange( min_value , max_value , delta )
        return vector

    def _strided_copy(self, *_):
        raise NotImplementedError()
    def _rshift(self, *_):
        raise NotImplementedError()
    def _lshift(self, *_):
        raise NotImplementedError()
    def _alloc(self, *_):
        raise NotImplementedError()
    def _element_size(self, *_):
        raise NotImplementedError()
    def _contains(self, *_):
        raise NotImplementedError()
    def _pop(self, *_):
        raise NotImplementedError()
    def default_format(self, *_):
        raise NotImplementedError()
    def _iget(self, *_):
        raise NotImplementedError()
    def _iset(self, *_):
        raise NotImplementedError()
    def _inplace_add(self, *_):
        raise NotImplementedError()
    def _shift(self, *_):
        raise NotImplementedError()
    def _alloc_copy(self, *_):
        raise NotImplementedError()
    def _inplace_mul(self, *_):
        raise NotImplementedError()
    def _scale(self, *_):
        raise NotImplementedError()
    def _memcpy(self, *_):
        raise NotImplementedError()
    def _assign(self, *_):
        raise NotImplementedError()
    def _size(self, *_):
        raise NotImplementedError()
    def _fprintf(self, *_):
        raise NotImplementedError()
    def _get_max(self, *_):
        raise NotImplementedError()
    def _get_min(self, *_):
        raise NotImplementedError()
    def _get_min_index(self, *_):
        raise NotImplementedError()
    def _get_max_index(self, *_):
        raise NotImplementedError()
    def _append(self, *_):
        raise NotImplementedError()
    def _idel_block(self, *_):
        raise NotImplementedError()
    def _sort(self, *_):
        raise NotImplementedError()
    def _rsort(self, *_):
        raise NotImplementedError()
    def _reset(self, *_):
        raise NotImplementedError()
    def _safe_iget(self, *_):
        raise NotImplementedError()
    def _set_read_only(self, *_):
        raise NotImplementedError()
    def _get_read_only(self, *_):
        raise NotImplementedError()
    def _set_default(self, *_):
        raise NotImplementedError()
    def _get_default(self, *_):
        raise NotImplementedError()
    def _free(self, *_):
        raise NotImplementedError()
    def _permute(self, *_):
        raise NotImplementedError()
    def _sort_perm(self, *_):
        raise NotImplementedError()
    def _rsort_perm(self, *_):
        raise NotImplementedError()
    def _select_unique(self, *_):
        raise NotImplementedError()
    def _element_sum(self, *_):
        raise NotImplementedError()
    def _get_data_ptr(self, *_):
        raise NotImplementedError()
    def _count_equal(self, *_):
        raise NotImplementedError()
    def _init_range(self, *_):
        raise NotImplementedError()

#  Copyright (C) 2014  Statoil ASA, Norway.
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

   print vec[1]

will give '66'. The main part of the implementation is in terms of an
"abstract base class" TVector. The TVector class should be not
instantiated directly, instead the child classes IntVector,
DoubleVector or BoolVector should be used. 

The C-level has implementations for several fundamental types like
float and size_t not currently implemented in the Python version.
"""

import  sys
from    types import IntType, SliceType

from ert.cwrap import CWrapper, CFILE, BaseCClass
from ert.util import UTIL_LIB


class PermutationVector(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def free(self):
        PermutationVector.cNamespace().free(self)

CWrapper.registerObjectType("permutation_vector", PermutationVector)

cwrapper = CWrapper(UTIL_LIB)
PermutationVector.cNamespace().free = cwrapper.prototype("void util_safe_free(permutation_vector)")



# TVector takes advantage of the fact that self.cNamespace belongs to the inheriting class
class VectorTemplate(BaseCClass):
    @classmethod
    def strided_copy(cls, obj, slice_range):
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
        (start, stop, step) = slice_range.indices(len(obj))
        if stop > start:
            return cls.cNamespace().strided_copy(obj, start, stop, step)
        else:
            return None


    @classmethod
    def __copy__(cls, obj):
        return cls.cNamespace().alloc_copy(obj)


    def copy(self):
        """
        Create a new copy of the current vector.
        """
        new = self.__copy__(self)  # Invoking the class method
        return new

    def __irshift__(self,shift):
        if shift < 0:
            raise ValueError("The shift must be positive")
        self.cNamespace().rshift(self, shift)
        return self
        
    def __ilshift__(self,shift):
        if shift < 0:
            raise ValueError("The shift must be positive")
        if shift > len(self):
            raise ValueError("The shift is too large %d > %d" % (shift, len(self)))
        self.cNamespace().lshift(self, shift)
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
        c_pointer = self.cNamespace().alloc(initial_size, default_value)
        super(VectorTemplate, self).__init__(c_pointer)
        self.element_size = self.cNamespace().element_size(self)
        
    def __contains__(self , value):
        return self.cNamespace().contains(self , value)


    def pop(self):
        if len(self) > 0:
            return self.cNamespace().pop(self)
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
        lines = len(self) / width
        if not fmt:
            fmt = self.default_format + " "

        if max_lines is None or lines <= max_lines:
            s += self.str_data(width, 0, len(self), fmt)
        else:
            s1 = width * max_lines / 2
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
        if isinstance(index, IntType):
            length = len(self)
            if index < 0:
                index += length

            if index < 0 or index >= length:
                raise IndexError("Index must be in range %d <= %d < %d" % (0, index, length))
            else:
                return self.cNamespace().iget(self, index)
        elif isinstance(index, SliceType):
            return self.strided_copy(self, index)
        else:
            raise TypeError("Index should be integer or slice type.")

    def __setitem__(self, index, value):
        """
        Implements write [] operator - @index must be integer.
        """
        if isinstance(index, IntType):
            self.cNamespace().iset(self, index, value)
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
                    self.cNamespace().scale(delta, -1)
                self.cNamespace().inplace_add(self, delta)
            else:
                raise ValueError("Incompatible sizes for add self:%d  other:%d" % (len(self), len(delta)))
        else:
            if isinstance(delta, int) or isinstance(delta, float):
                if not add:
                    delta *= -1
                self.cNamespace().shift(self, delta)
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
        copy = self.__copy__(self)
        copy += delta
        return copy

    def __sub__(self, delta):
        """
        Implements subtraction - creating new copy.
        """
        copy = self.__copy__(self)
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
                self.cNamespace().inplace_mul(self, factor)
            else:
                raise ValueError("Incompatible sizes for mul self:%d  other:%d" % (len(self), len(factor)))
        else:
            if isinstance(factor, int) or isinstance(factor, float):
                self.cNamespace().scale(self, factor)
            else:
                raise TypeError("factor has wrong type:%s " % type(factor))

        return self


    def __mul__(self, factor):
        copy = self.__copy__(self)
        copy *= factor
        return copy

    def __rmul__(self, factor):
        return self.__mul__(factor)


    def __div__(self, divisor):
        if isinstance(divisor, int) or isinstance(divisor, float):
            copy = self.__copy__(self)
            copy.cNamespace().div(copy, divisor)
            return copy
        else:
            raise TypeError("Divisor has wrong type:%s" % type(divisor))

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
            self.cNamespace().memcpy(self, value)
        else:
            if isinstance(value, int) or isinstance(value, float):
                self.cNamespace().assign(self, value)
            else:
                raise TypeError("Value has wrong type")

    def __len__(self):
        """
        The number of elements in the vector.
        """
        return self.cNamespace().size(self)


    def printf(self, fmt=None, name=None, stream=sys.stdout):
        """
        See also the str() method which returns string representantion
        of the vector.
        """
        cfile = CFILE(stream)
        if not fmt:
            fmt = self.default_format
        self.cNamespace().fprintf(self, cfile, name, fmt)


    def max(self):
        if len(self) > 0:
            return self.cNamespace().get_max(self)
        else:
            raise IndexError("The vector is empty!")

    def min(self):
        if len(self) > 0:
            return self.cNamespace().get_min(self)
        else:
            raise IndexError("The vector is empty!")


    def minIndex(self, reverse=False):
        """
        @type reverse: bool
        @rtype: int
        """
        if len(self) > 0:
            return self.cNamespace().get_min_index(self, reverse)
        else:
            raise IndexError("The vector is empty!")

    def maxIndex(self, reverse=False):
        """
        @type reverse: bool
        @rtype: int
        """
        if len(self) > 0:
            return self.cNamespace().get_max_index(self, reverse)
        else:
            raise IndexError("The vector is empty!")

    def append(self, value):
        self.cNamespace().append(self, value)

    def deleteBlock(self, index, block_size):
        """
        Remove a block of size @block_size starting at @index.
        
        After the removal data will be left shifted.
        """
        self.cNamespace().idel_block(self, index, block_size)

    def sort(self, reverse=False):
        """
        Sort the vector inplace in increasing numerical order or decreasing order if reverse is True.
        @type reverse: bool
        """
        if not reverse:
            self.cNamespace().sort(self)
        else:
            self.cNamespace().rsort(self)

    def clear(self):
        self.cNamespace().reset(self)

    def safeGetByIndex(self, index):
        return self.cNamespace().safe_iget(self, index)

    def setReadOnly(self, read_only):
        self.cNamespace().set_read_only(self, read_only)

    def getReadOnly(self):
        return self.cNamespace().get_read_only(self)

    def setDefault(self, value):
        self.cNamespace().set_default(self, value)

    def getDefault(self):
        return self.cNamespace().get_default(self)


    def free(self):
        self.cNamespace().free(self)

    def permute(self, permutation_vector):
        """
        Reorders this vector based on the indexes in permutation_vector.
        @type permutation_vector: PermutationVector
        """
        assert isinstance(permutation_vector, PermutationVector)

        self.cNamespace().permute(self, permutation_vector)

    def permutationSort(self, reverse=False):
        """
        Returns the permutation vector for sorting of this vector. Vector is not sorted.
         @type reverse: bool
         @@rtype: PermutationVector
        """
        if len(self) > 0:
            if not reverse:
                return self.cNamespace().sort_perm(self)
            else:
                return self.cNamespace().rsort_perm(self)

        return None


    def asList(self):
        l = [0] * len(self)
        for (index,value) in enumerate(self):
            l[index] = value
            
        return l

    def selectUnique(self):
        self.cNamespace().select_unique(self)


    def elementSum(self):
        return self.cNamespace().element_sum( self )


    def getDataPtr(self):
        "Low level function which returns a pointer to underlying storage"
        # Observe that the get_data_ptr() function is not implemented
        # for the TimeVector class.
        return self.cNamespace().get_data_ptr(self)

    def countEqual(self , value):
        return self.cNamespace().count_equal( self , value )


    def initRange(self , min_value , max_value , delta):
        """
        Will fill the vector with the values from min_value to
        max_value in steps of delta. The upper limit is guaranteed to
        be inclusive, even if it is not commensurable with the delta.
        """
        if delta == 0:
            raise ValueError("Invalid range")
        else:
            self.cNamespace().init_range( self , min_value , max_value , delta )

#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'tvector.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import  ctypes
import  numpy

from ert.cwrap import CWrapper, CFILE, BaseCClass
from ert.util import UTIL_LIB, ctime




#TVector takes advantage of the fact that self.cNamespace belongs to the inheriting class
class TVector(BaseCClass):

    @classmethod
    def strided_copy( cls , obj , slice_range ):
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
        (start , stop , step) = slice_range.indices( obj.size )
        if stop > start:
            return cls.cNamespace().strided_copy(obj, start, stop, step)
        else:
            return None


    @classmethod
    def __copy__( cls , obj ):
        return cls.cNamespace().alloc_copy(obj)


    def copy( self ):
        """
        Create a new copy of the current vector.
        """
        new = self.__copy__( self )  # Invoking the class method
        return new


    def __deepcopy__(self , memo):
        new = self.copy(  )
        return new

    def __init__(self, default_value = 0, initial_size = 0):
        """
        Creates a new TVector instance.
        """
        c_pointer = self.cNamespace().alloc(initial_size, default_value)
        super(TVector, self).__init__(c_pointer)
        self.element_size = self.cNamespace().element_size( self )
        

    def str_data( self , width , index1 , index2 , fmt):
        """
        Helper function for str() method.
        """
        data = []
        s = ""
        for index in range(index1, index2):
            data.append( self[index] )
        for index in range(len(data)):
            s += fmt % data[ index ]
            if index % width == (width - 1):
                s+= "\n"
        return s


    # The str() method is a verbatim copy of the implementation in
    # ecl_kw.py.
    def str(self , width = 5 , max_lines = 10 , fmt = None):
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
        lines = self.size / width
        if not fmt:
            fmt = self.def_fmt + " "

        if max_lines is None or lines <= max_lines:
            s += self.str_data( width , 0 , self.size , fmt)
        else:
            s1 = width * max_lines / 2
            s += self.str_data( width  , 0 , s1 , fmt)
            s += "   ....   \n"
            s += self.str_data( width  , self.size - s1 , self.size , fmt)

        return s

    def __str__(self):
        """
        Returns string representantion of vector.
        """
        return self.str( max_lines = 10 , width = 5 )



    def __getitem__(self, index ):
        """
        Implements read [] operator - @index can be slice instance.
        """
        if isinstance( index , IntType):
            length = self.__len__()
            if index < 0:
                index += length

            if index < 0 or index >= length:
                raise IndexError
            else:
                return self.cNamespace().iget( self , index )
        elif isinstance( index , SliceType ):
            return self.strided_copy( self , index )
        else:
            raise TypeError("Index should be integer or slice type.")

    def __setitem__( self , index , value ):
        """
        Implements write [] operator - @index must be integer.
        """
        if isinstance( index , IntType):
            self.cNamespace().iset( self, index , value )
        else:
            raise TypeError("Index should be integer type")

    ##################################################################
    # Mathematical operations:

    def __IADD__(self , delta , add ):
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
            if self.size == delta.size:
                # This is vector + vector operation. 
                if not add:
                    self.cNamespace().scale( delta , -1 )
                self.cNamespace().inplace_add( self , delta )
            else:
                raise ValueError("Incompatible sizes for add self:%d  other:%d" % (self.size , delta.size))
        else:
            if isinstance( delta , int ) or isinstance( delta, float):
                if not add:
                    delta *= -1
                self.cNamespace().shift( self , delta )
            else:
                raise TypeError("delta has wrong type:%s " % type(delta))

        return self

    # the __IMUL__ function only implements multiplication.
    def __IMUL__(self , factor ):
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
            if self.size == factor.size:
                self.cNamespace().inplace_mul( self , factor  )
            else:
                raise ValueError("Incompatible sizes for mul self:%d  other:%d" % (self.size , factor.size))
        else:
            if isinstance( factor , int ) or isinstance( factor, float):
                self.cNamespace().scale( self , factor )
            else:
                raise TypeError("factor has wrong type:%s " % type(factor))

        return self


    def __iadd__(self , delta ):
        """
        Implements inplace add. @delta can be vector or scalar.
        """
        return self.__IADD__( delta , True )

    def __isub__(self , delta):
        """
        Implements inplace subtract. @delta can be vector or scalar.
        """
        return self.__IADD__(delta , False )

    def __radd__(self , delta):
        return self.__add__( delta )

    def __add__(self , delta):
        """
        Implements add operation - creating a new copy.

           b = DoubleVector()
           c = DoubleVector()  // Or alternatively scalar
           ....
           a = b + c
        """
        copy = self.__copy__( self )
        copy += delta
        return copy

    def __sub__(self , delta):
        """
        Implements subtraction - creating new copy.
        """
        copy  = self.__copy__( self )
        copy -= delta
        return copy

    def __rsub__( self , delta):
        return self.__sub__( delta ) * -1

    def __imul__(self , factor ):
        return self.__IMUL__( factor )

    def __mul__( self , factor ):
        copy = self.__copy__( self )
        copy *= factor
        return copy

    def __rmul__(self , factor):
        return self.__mul__( factor )


    def __div__(self , divisor):
        if isinstance( divisor , int ) or isinstance( divisor , float):
            copy = self.__copy__( self )
            copy.cNamespace().div( copy , divisor )
            return copy
        else:
            raise TypeError("Divisor has wrong type:%s" % type( divisor ))

    # No __rdiv__()


    # End mathematical operations
    #################################################################

    # Essentally implements a = b
    def assign(self , value):
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
            self.cNamespace().memcpy( self , value )
        else:
            if isinstance( value , int ) or isinstance( value, float):
                self.cNamespace().assign( self , value )
            else:
                raise TypeError("Value has wrong type")

    def __len__(self):
        """
        The number of elements in the vector.
        """
        return self.cNamespace().size( self )


    def printf( self , fmt = None , name = None , stream = sys.stdout ):
        """
        See also the str() method which returns string representantion
        of the vector.
        """
        cfile = CFILE( stream )
        if not fmt:
            fmt = self.def_fmt
        self.cNamespace().fprintf(self , cfile , name , fmt)

    @property
    def size( self ):
        """
        The number of elements in the vector.
        """
        return self.__len__()

    @property
    def max( self ):
        if self.cNamespace().size( self ) > 0:
            return self.cNamespace().get_max( self )
        else:
            raise IndexError

    @property
    def min( self ):
        if self.cNamespace().size( self ) > 0:
            return self.cNamespace().get_min( self )
        else:
            raise IndexError


    def min_index( self , reverse = False ):
        if self.cNamespace().size( self ) > 0:
            return self.cNamespace().get_min_index( self , reverse )
        else:
            raise IndexError

    def max_index( self , reverse = False ):
        if self.cNamespace().size( self ) > 0:
            return self.cNamespace().get_max_index( self , reverse )
        else:
            raise IndexError

    def append( self , value ):
        self.cNamespace().append( self , value )

    def del_block( self , index , block_size ):
        """
        Remove a block of size @block_size starting at @index.
        
        After the removal data will be left shifted.
        """
        self.cNamespace().idel_block( self , index , block_size )

    def sort( self ):
        """
        Sort the vector inplace in increasing numerical order.
        """
        self.cNamespace().sort( self )

    def rsort( self ):
        """
        Sort the vector inplace in reverse (decreasing) numerical order.
        """
        self.cNamespace().rsort( self )

    def clear(self):
        self.cNamespace().reset( self )

    def safe_iget( self , index):
        return self.cNamespace().safe_iget( self , index )

    def set_read_only( self , read_only ):
        self.cNamespace().set_read_only( self , read_only )

    def get_read_only( self ):
        return self.cNamespace().get_read_only( self )

    read_only = property( get_read_only , set_read_only )

    def set_default( self , value ):
        self.cNamespace().set_default( self , value )

    def get_default( self ):
        return self.cNamespace().get_default( self )

    default = property( get_default , set_default )


    def free(self):
        self.cNamespace().free(self)

    # The numpy_copy() method goes thorugh a bit of hoops to get the
    # memory ownership correct:
    #
    #   1. A numpy array (view) is created which wraps the underlying
    #      storage of the vector instance.
    #
    #   2. A new numpy copy is created from the view and returned.
    #
    # The point of this involved exercise is to ensure that the numpy
    # copy has it's own storage copy[1], and to ensure that the numpy
    # storage is disposed correctly when the numpy vector is garbage
    # collected.
    #
    # 1: The underlying storage of the vector instance is quite
    #    volatile, and it is not possible to create a numpy instance
    #    which safely shares storage with the vector.

    def numpy_copy(self):
        """
        Will return a copy of the vector as a numpy array.

        The returned numpy copy is a true copy, and does not share any
        storage with the vector instance.
        """
        data_ptr = self.cNamespace().data_ptr( self )
        buffer_size = self.size * self.element_size
        buffer = buffer_from_ptr( data_ptr , buffer_size )
        view = numpy.frombuffer( buffer , self.numpy_dtype )
        return numpy.copy( view )


#################################################################

# The ctypes type system with CWrapper.registerType() needs access to
# the wrapper object class definitions, and the warpper objects need
# access to the cfunc.xxxx function objects; that is the reason we
# invoke the ugly cls.initialized flag.

class DoubleVector(TVector):
    def __init__(self, default_value=0, initial_size=0):
        super(DoubleVector, self).__init__(default_value, initial_size)


class BoolVector(TVector):
    def __init__(self, default_value=0, initial_size=0):
        super(BoolVector, self).__init__(default_value, initial_size)

    @classmethod
    def active_mask(cls , range_string):
        """
        Will create a BoolVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {F,T,F,F,T,T,T,T,F,F,T}
           "1,4-7,10X" =>  {}
        
        The empty list will evaluate to false
        """
        return cls.cNamespace().create_active_mask(range_string)

    @classmethod
    def create_from_list(cls, size, source_list):
        """Allocates a bool vector from a Python list"""
        bool_vector = BoolVector(False, size)

        for index in range(len(source_list)):
            bool_vector[index] = source_list[index]

        return bool_vector






class IntVector(TVector):


    def __init__(self, default_value=0, initial_size=0):
        super(IntVector, self).__init__(default_value, initial_size)

    @classmethod
    def active_list(cls , range_string):
        """
        Will create a IntVector instance with the values from @range_string.

        The range_string input should be of the type "1,3-5,9,17",
        i.e. integer values separated by commas, and dashes to
        represent ranges. If the input string contains ANY invalid
        characters the returned active list will be empty:

           "1,4-7,10"  =>  {1,4,5,6,7,10}
           "1,4-7,10X" =>  {}
        
        The empty list will evaluate to false
        """
        return cls.cNamespace().create_active_list(range_string)


class TimeVector(TVector):
    def __init__(self, default_value=0, initial_size=0):
        super(TimeVector, self).__init__(default_value, initial_size)


#################################################################

buffer_from_ptr = ctypes.pythonapi.PyBuffer_FromMemory
buffer_from_ptr.restype  = ctypes.py_object
buffer_from_ptr.argtypes = [ ctypes.c_void_p , ctypes.c_long ]

CWrapper.registerType("double_vector", DoubleVector)
CWrapper.registerType("double_vector_obj", DoubleVector.createPythonObject)
CWrapper.registerType("double_vector_ref", DoubleVector.createCReference)

CWrapper.registerType("int_vector", IntVector)
CWrapper.registerType("int_vector_obj", IntVector.createPythonObject)
CWrapper.registerType("int_vector_ref", IntVector.createCReference)

CWrapper.registerType("bool_vector", BoolVector)
CWrapper.registerType("bool_vector_obj", BoolVector.createPythonObject)
CWrapper.registerType("bool_vector_ref", BoolVector.createCReference)

CWrapper.registerType("time_t_vector", TimeVector)
CWrapper.registerType("time_t_vector_obj", TimeVector.createPythonObject)
CWrapper.registerType("time_t_vector_ref", TimeVector.createCReference)


cwrapper = CWrapper(UTIL_LIB)



DoubleVector.cNamespace().alloc            = cwrapper.prototype("c_void_p   double_vector_alloc( int , double )")
DoubleVector.cNamespace().alloc_copy       = cwrapper.prototype("double_vector_obj   double_vector_alloc_copy( double_vector )")
DoubleVector.cNamespace().strided_copy     = cwrapper.prototype("double_vector_obj   double_vector_alloc_strided_copy( double_vector , int , int , int)")
DoubleVector.cNamespace().free             = cwrapper.prototype("void   double_vector_free( double_vector )")
DoubleVector.cNamespace().iget             = cwrapper.prototype("double double_vector_iget( double_vector , int )")
DoubleVector.cNamespace().safe_iget        = cwrapper.prototype("double double_vector_safe_iget( int_vector , int )")
DoubleVector.cNamespace().iset             = cwrapper.prototype("double double_vector_iset( double_vector , int , double)")
DoubleVector.cNamespace().size             = cwrapper.prototype("int    double_vector_size( double_vector )")
DoubleVector.cNamespace().append           = cwrapper.prototype("void   double_vector_append( double_vector , double )")
DoubleVector.cNamespace().idel_block       = cwrapper.prototype("void   double_vector_idel_block( double_vector , int , int )")
DoubleVector.cNamespace().fprintf          = cwrapper.prototype("void   double_vector_fprintf( double_vector , FILE , char* , char*)")
DoubleVector.cNamespace().sort             = cwrapper.prototype("void   double_vector_sort( double_vector )")
DoubleVector.cNamespace().rsort            = cwrapper.prototype("void   double_vector_rsort( double_vector )")
DoubleVector.cNamespace().reset            = cwrapper.prototype("void   double_vector_reset( double_vector )")
DoubleVector.cNamespace().get_read_only    = cwrapper.prototype("bool   double_vector_get_read_only( double_vector )")
DoubleVector.cNamespace().set_read_only    = cwrapper.prototype("void   double_vector_set_read_only( double_vector , bool )")
DoubleVector.cNamespace().get_max          = cwrapper.prototype("double    double_vector_get_max( double_vector )")
DoubleVector.cNamespace().get_min          = cwrapper.prototype("double    double_vector_get_min( double_vector )")
DoubleVector.cNamespace().get_max_index    = cwrapper.prototype("int    double_vector_get_max_index( double_vector , bool)")
DoubleVector.cNamespace().get_min_index    = cwrapper.prototype("int    double_vector_get_min_index( double_vector , bool)")
DoubleVector.cNamespace().shift            = cwrapper.prototype("void   double_vector_shift( double_vector , double )")
DoubleVector.cNamespace().scale            = cwrapper.prototype("void   double_vector_scale( double_vector , double )")
DoubleVector.cNamespace().div              = cwrapper.prototype("void   double_vector_div( double_vector , double )")
DoubleVector.cNamespace().inplace_add      = cwrapper.prototype("void   double_vector_inplace_add( double_vector , double_vector )")
DoubleVector.cNamespace().inplace_mul      = cwrapper.prototype("void   double_vector_inplace_mul( double_vector , double_vector )")
DoubleVector.cNamespace().assign              = cwrapper.prototype("void   double_vector_set_all( double_vector , double)")
DoubleVector.cNamespace().memcpy              = cwrapper.prototype("void   double_vector_memcpy(double_vector , double_vector )")
DoubleVector.cNamespace().set_default         = cwrapper.prototype("void   double_vector_set_default( double_vector , double)")
DoubleVector.cNamespace().get_default         = cwrapper.prototype("double    double_vector_get_default( double_vector )")
DoubleVector.cNamespace().alloc_data_copy     = cwrapper.prototype("double*  double_vector_alloc_data_copy( double_vector )")
DoubleVector.cNamespace().data_ptr            = cwrapper.prototype("double*  double_vector_get_ptr( double_vector )")
DoubleVector.cNamespace().element_size        = cwrapper.prototype("int      double_vector_element_size( double_vector )")


IntVector.cNamespace().alloc               = cwrapper.prototype("c_void_p   int_vector_alloc( int , int )")
IntVector.cNamespace().alloc_copy          = cwrapper.prototype("int_vector_obj int_vector_alloc_copy( int_vector )")
IntVector.cNamespace().strided_copy        = cwrapper.prototype("int_vector_obj int_vector_alloc_strided_copy( int_vector , int , int , int)")
IntVector.cNamespace().free                = cwrapper.prototype("void   int_vector_free( int_vector )")
IntVector.cNamespace().iget                = cwrapper.prototype("int    int_vector_iget( int_vector , int )")
IntVector.cNamespace().safe_iget           = cwrapper.prototype("int    int_vector_safe_iget( int_vector , int )")
IntVector.cNamespace().iset                = cwrapper.prototype("int    int_vector_iset( int_vector , int , int)")
IntVector.cNamespace().size                = cwrapper.prototype("int    int_vector_size( int_vector )")
IntVector.cNamespace().append              = cwrapper.prototype("void   int_vector_append( int_vector , int )")
IntVector.cNamespace().idel_block          = cwrapper.prototype("void   int_vector_idel_block( int_vector , int , int )")
IntVector.cNamespace().fprintf             = cwrapper.prototype("void   int_vector_fprintf( int_vector , FILE , char* , char*)")
IntVector.cNamespace().sort                = cwrapper.prototype("void   int_vector_sort( int_vector )")
IntVector.cNamespace().rsort               = cwrapper.prototype("void   int_vector_rsort( int_vector )")
IntVector.cNamespace().reset               = cwrapper.prototype("void   int_vector_reset( int_vector )")
IntVector.cNamespace().set_read_only       = cwrapper.prototype("void   int_vector_set_read_only( int_vector , bool )")
IntVector.cNamespace().get_read_only       = cwrapper.prototype("bool   int_vector_get_read_only( int_vector )")
IntVector.cNamespace().get_max             = cwrapper.prototype("int    int_vector_get_max( int_vector )")
IntVector.cNamespace().get_min             = cwrapper.prototype("int    int_vector_get_min( int_vector )")
IntVector.cNamespace().get_max_index       = cwrapper.prototype("int    int_vector_get_max_index( int_vector , bool)")
IntVector.cNamespace().get_min_index       = cwrapper.prototype("int    int_vector_get_min_index( int_vector , bool)")
IntVector.cNamespace().shift               = cwrapper.prototype("void   int_vector_shift( int_vector , int )")
IntVector.cNamespace().scale               = cwrapper.prototype("void   int_vector_scale( int_vector , int )")
IntVector.cNamespace().div                 = cwrapper.prototype("void   int_vector_div( int_vector , int )")
IntVector.cNamespace().inplace_add         = cwrapper.prototype("void   int_vector_inplace_add( int_vector , int_vector )")
IntVector.cNamespace().inplace_mul         = cwrapper.prototype("void   int_vector_inplace_mul( int_vector , int_vector )")
IntVector.cNamespace().assign              = cwrapper.prototype("void   int_vector_set_all( int_vector , int)")
IntVector.cNamespace().memcpy              = cwrapper.prototype("void   int_vector_memcpy(int_vector , int_vector )")
IntVector.cNamespace().set_default         = cwrapper.prototype("void   int_vector_set_default( int_vector , int)")
IntVector.cNamespace().get_default         = cwrapper.prototype("int    int_vector_get_default( int_vector )")
IntVector.cNamespace().alloc_data_copy     = cwrapper.prototype("int*  int_vector_alloc_data_copy( int_vector )")
IntVector.cNamespace().data_ptr            = cwrapper.prototype("int*  int_vector_get_ptr( int_vector )")
IntVector.cNamespace().element_size        = cwrapper.prototype("int    int_vector_element_size( int_vector )")

IntVector.cNamespace().create_active_list = cwrapper.prototype("int_vector_obj string_util_alloc_active_list( char* )")


BoolVector.cNamespace().alloc               = cwrapper.prototype("c_void_p   bool_vector_alloc( int , bool )")
BoolVector.cNamespace().alloc_copy          = cwrapper.prototype("bool_vector_obj bool_vector_alloc_copy( bool_vector )")
BoolVector.cNamespace().strided_copy        = cwrapper.prototype("bool_vector_obj bool_vector_alloc_strided_copy( bool_vector , bool , bool , bool)")
BoolVector.cNamespace().free                = cwrapper.prototype("void   bool_vector_free( bool_vector )")
BoolVector.cNamespace().iget                = cwrapper.prototype("bool    bool_vector_iget( bool_vector , bool )")
BoolVector.cNamespace().safe_iget           = cwrapper.prototype("bool    bool_vector_safe_iget( bool_vector , bool )")
BoolVector.cNamespace().iset                = cwrapper.prototype("bool    bool_vector_iset( bool_vector , bool , bool)")
BoolVector.cNamespace().size                = cwrapper.prototype("bool    bool_vector_size( bool_vector )")
BoolVector.cNamespace().append              = cwrapper.prototype("void   bool_vector_append( bool_vector , bool )")
BoolVector.cNamespace().idel_block          = cwrapper.prototype("void   bool_vector_idel_block( bool_vector , bool , bool )")
BoolVector.cNamespace().fprintf             = cwrapper.prototype("void   bool_vector_fprintf( bool_vector , FILE , char* , char*)")
BoolVector.cNamespace().sort                = cwrapper.prototype("void   bool_vector_sort( bool_vector )")
BoolVector.cNamespace().rsort               = cwrapper.prototype("void   bool_vector_rsort( bool_vector )")
BoolVector.cNamespace().reset               = cwrapper.prototype("void   bool_vector_reset( bool_vector )")
BoolVector.cNamespace().set_read_only       = cwrapper.prototype("void   bool_vector_set_read_only( bool_vector , bool )")
BoolVector.cNamespace().get_read_only       = cwrapper.prototype("bool   bool_vector_get_read_only( bool_vector )")
BoolVector.cNamespace().get_max             = cwrapper.prototype("bool    bool_vector_get_max( bool_vector )")
BoolVector.cNamespace().get_min             = cwrapper.prototype("bool    bool_vector_get_min( bool_vector )")
BoolVector.cNamespace().get_max_index       = cwrapper.prototype("bool    bool_vector_get_max_index( bool_vector , bool)")
BoolVector.cNamespace().get_min_index       = cwrapper.prototype("bool    bool_vector_get_min_index( bool_vector , bool)")
BoolVector.cNamespace().shift               = cwrapper.prototype("void   bool_vector_shift( bool_vector , bool )")
BoolVector.cNamespace().scale               = cwrapper.prototype("void   bool_vector_scale( bool_vector , bool )")
BoolVector.cNamespace().div                 = cwrapper.prototype("void   bool_vector_div( bool_vector , bool )")
BoolVector.cNamespace().inplace_add         = cwrapper.prototype("void   bool_vector_inplace_add( bool_vector , bool_vector )")
BoolVector.cNamespace().inplace_mul         = cwrapper.prototype("void   bool_vector_inplace_mul( bool_vector , bool_vector )")
BoolVector.cNamespace().assign              = cwrapper.prototype("void   bool_vector_set_all( bool_vector , bool)")
BoolVector.cNamespace().memcpy              = cwrapper.prototype("void   bool_vector_memcpy(bool_vector , bool_vector )")
BoolVector.cNamespace().set_default         = cwrapper.prototype("void   bool_vector_set_default( bool_vector , bool)")
BoolVector.cNamespace().get_default         = cwrapper.prototype("bool   bool_vector_get_default( bool_vector )")
BoolVector.cNamespace().alloc_data_copy     = cwrapper.prototype("bool*  bool_vector_alloc_data_copy( bool_vector )")
BoolVector.cNamespace().data_ptr            = cwrapper.prototype("bool*  bool_vector_get_ptr( bool_vector )")
BoolVector.cNamespace().element_size        = cwrapper.prototype("int    bool_vector_element_size( bool_vector )")

BoolVector.cNamespace().create_active_mask = cwrapper.prototype("bool_vector_obj string_util_alloc_active_mask( char* )")


TimeVector.cNamespace().alloc               = cwrapper.prototype("c_void_p time_t_vector_alloc(int, time_t )")
TimeVector.cNamespace().alloc_copy          = cwrapper.prototype("time_t_vector_obj time_t_vector_alloc_copy(time_t_vector )")
TimeVector.cNamespace().strided_copy        = cwrapper.prototype("time_t_vector_obj time_t_vector_alloc_strided_copy(time_t_vector , time_t , time_t , time_t)")
TimeVector.cNamespace().free                = cwrapper.prototype("void   time_t_vector_free( time_t_vector )")
TimeVector.cNamespace().iget                = cwrapper.prototype("time_t   time_t_vector_iget( time_t_vector , int )")
TimeVector.cNamespace().safe_iget           = cwrapper.prototype("time_t   time_t_vector_safe_iget( time_t_vector , int )")
TimeVector.cNamespace().iset                = cwrapper.prototype("time_t   time_t_vector_iset( time_t_vector , int , time_t)")
TimeVector.cNamespace().size                = cwrapper.prototype("int      time_t_vector_size( time_t_vector )")
TimeVector.cNamespace().append              = cwrapper.prototype("void     time_t_vector_append( time_t_vector , time_t )")
TimeVector.cNamespace().idel_block          = cwrapper.prototype("void     time_t_vector_idel_block( time_t_vector , int , int )")
TimeVector.cNamespace().fprintf             = cwrapper.prototype("void     time_t_vector_fprintf( time_t_vector , FILE , char* , char*)")
TimeVector.cNamespace().sort                = cwrapper.prototype("void     time_t_vector_sort( time_t_vector )")
TimeVector.cNamespace().rsort               = cwrapper.prototype("void     time_t_vector_rsort( time_t_vector )")
TimeVector.cNamespace().reset               = cwrapper.prototype("void     time_t_vector_reset( time_t_vector )")
TimeVector.cNamespace().set_read_only       = cwrapper.prototype("void     time_t_vector_set_read_only( time_t_vector , bool )")
TimeVector.cNamespace().get_read_only       = cwrapper.prototype("bool     time_t_vector_get_read_only( time_t_vector )")
TimeVector.cNamespace().get_max             = cwrapper.prototype("time_t   time_t_vector_get_max( time_t_vector )")
TimeVector.cNamespace().get_min             = cwrapper.prototype("time_t   time_t_vector_get_min( time_t_vector )")
TimeVector.cNamespace().get_max_index       = cwrapper.prototype("int      time_t_vector_get_max_index( time_t_vector , bool)")
TimeVector.cNamespace().get_min_index       = cwrapper.prototype("int      time_t_vector_get_min_index( time_t_vector , bool)")
TimeVector.cNamespace().shift               = cwrapper.prototype("void     time_t_vector_shift( time_t_vector , time_t )")
TimeVector.cNamespace().scale               = cwrapper.prototype("void     time_t_vector_scale( time_t_vector , time_t )")
TimeVector.cNamespace().div                 = cwrapper.prototype("void     time_t_vector_div( time_t_vector , time_t )")
TimeVector.cNamespace().inplace_add         = cwrapper.prototype("void     time_t_vector_inplace_add( time_t_vector , time_t_vector )")
TimeVector.cNamespace().inplace_mul         = cwrapper.prototype("void     time_t_vector_inplace_mul( time_t_vector , time_t_vector )")
TimeVector.cNamespace().assign              = cwrapper.prototype("void     time_t_vector_set_all( time_t_vector , time_t)")
TimeVector.cNamespace().memcpy              = cwrapper.prototype("void     time_t_vector_memcpy(time_t_vector , time_t_vector )")
TimeVector.cNamespace().set_default         = cwrapper.prototype("void     time_t_vector_set_default( time_t_vector , time_t)")
TimeVector.cNamespace().get_default         = cwrapper.prototype("time_t   time_t_vector_get_default( time_t_vector )")
TimeVector.cNamespace().alloc_data_copy     = cwrapper.prototype("time_t*  time_t_vector_alloc_data_copy( time_t_vector )")
TimeVector.cNamespace().data_ptr            = cwrapper.prototype("time_t*  time_t_vector_get_ptr( time_t_vector )")
TimeVector.cNamespace().element_size        = cwrapper.prototype("int      time_t_vector_element_size( time_t_vector )")

#-----------------------------------------------------------------





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
import  types
import  ctypes
import  libutil
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cfile       import CFILE
from    ert.cwrap.cclass      import CClass
import  numpy


class TVector(CClass):
    
    @classmethod
    def strided_copy( cls , obj , slice ):
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
        (start , stop , step) = slice.indices( obj.size )
        if stop > start:
            new_obj = TVector.__new__( cls )
            c_ptr = cls.cstrided_copy( obj  , start , stop , step )
            new_obj.init_cobj( c_ptr , new_obj.free )
            return new_obj
        else:
            return None


    @classmethod
    def __copy__( cls , obj ):
        new_obj = TVector.__new__( cls )
        c_ptr = cls.alloc_copy( obj )
        new_obj.init_cobj( c_ptr , new_obj.free )
        return new_obj
    

    def __new__( cls ):
        obj = object.__new__( cls )
        return obj


    @classmethod
    def ref( cls , c_ptr , parent ):
        obj = cls( )
        obj.init_cref( c_ptr , parent )
        return obj

    
    def copy( self ):
        """
        Create a new copy of the current vector.
        """
        new = self.__copy__( self )  # Invoking the class method
        return new


    def __deepcopy__(self , memo):
        new = self.copy(  )
        return new

    def __init__( self , default_value = 0):
        """
        Creates a new TVector instance.
        """
        init_size  = 0
        c_ptr = self.alloc( init_size , default_value )
        self.init_cobj( c_ptr , self.free )
        self.element_size = self.get_element_size( self ) 


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
        if isinstance( index , types.IntType):
            length = self.__len__()
            if index < 0:
                index += length

            if index < 0 or index >= length:
                raise IndexError
            else:
                return self.iget( self , index )
        elif isinstance( index , types.SliceType ):
            return self.strided_copy( self , index )
        else:
            raise TypeError("Index should be integer or slice type.")
        
    def __setitem__( self , index , value ):
        """
        Implements write [] operator - @index must be integer.
        """
        if isinstance( index , types.IntType):
            self.iset( self, index , value )
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
            # This is vector + vector operation. 
            if not add:
                self.scale( delta , -1 )
            self.inplace_add( self , delta )
        else:
            if isinstance( delta , int ) or isinstance( delta, float):
                if not add:
                    delta *= -1
                self.shift( self , delta )
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
            self.inplace_mul( self , factor  )
        else:
            if isinstance( factor , int ) or isinstance( factor, float):
                self.scale( self , factor )
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
    
    def __div__(self , factor):
        copy = self.deep_copy()
        copy /= factor
        return copy
    
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
            self.memcpy( self , value )
        else:
            if isinstance( value , int ) or isinstance( value, float):
                self.cassign( self , value )
            else:
                raise TypeError("Value has wrong type")

    def __len__(self):
        """
        The number of elements in the vector."
        """
        return self.get_size( self )
    

    def printf( self , fmt = None , name = None , stream = sys.stdout ):
        """
        See also the str() method which returns string representantion
        of the vector.
        """
        cfile = CFILE( stream )
        if not fmt:
            fmt = self.def_fmt
        self.fprintf(self , cfile , name , fmt)

    @property
    def size( self ):
        """
        The number of elements in the vector.
        """
        return self.__len__()

    @property
    def max( self ):
        if self.get_size( self ) > 0:
            return self.get_max( self )
        else:
            raise IndexError

    @property
    def min( self ):
        if self.get_size( self ) > 0:
            return self.get_min( self )
        else:
            raise IndexError
        
    def min_index( self , reverse = False ):
        if self.get_size( self ) > 0:
            return self.get_min_index( self , reverse )
        else:
            raise IndexError

    def max_index( self , reverse = False ):
        if self.get_size( self ) > 0:
            return self.get_max_index( self , reverse )
        else:
            raise IndexError

    def append( self , value ):
        self.cappend( self , value )

    def del_block( self , index , block_size ):
        """
        Remove a block of size @block_size starting at @index.
        
        After the removal data will be left shifted.
        """
        self.idel_block( self , index , block_size )

    def sort( self ):
        """
        Sort the vector inplace in increasing numerical order.
        """
        self.csort( self )

    def rsort( self ):
        """
        Sort the vector inplace in reverse (decreasing) numerical order.
        """
        self.crsort( self )

    def clear(self):
        self.cclear( self )

    def safe_iget( self , index):
        return self.csafe_iget( self , index )

    def set_read_only( self , read_only ):
        self.set_read_only( self , read_only )

    def get_read_only( self ):
        return self.get_read_only( self )
        
    read_only = property( get_read_only , set_read_only )

    def set_default( self , value ):
        self.cset_default( self , value )

    def get_default( self ):
        return self.cget_default( self )

    default = property( get_default , set_default )


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
        data_ptr = self.data_ptr( self )
        buffer_size = self.size * self.element_size
        buffer = buffer_from_ptr( data_ptr , buffer_size )
        view = numpy.frombuffer( buffer , self.numpy_dtype )
        return numpy.copy( view )


#################################################################


class DoubleVector(TVector):
    initialized = False

    def __new__( cls , *arglist ):
        if not cls.initialized:
            cls.csort         = cfunc.double_vector_sort
            cls.crsort        = cfunc.double_vector_rsort
            cls.alloc         = cfunc.double_vector_alloc
            cls.alloc_copy    = cfunc.double_vector_alloc_copy
            cls.free          = cfunc.double_vector_free
            cls.get_size      = cfunc.double_vector_size
            cls.iget          = cfunc.double_vector_iget
            cls.iset          = cfunc.double_vector_iset
            cls.fprintf       = cfunc.double_vector_fprintf
            cls.cappend       = cfunc.double_vector_append
            cls.idel_block    = cfunc.double_vector_idel_block            
            cls.cclear        = cfunc.double_vector_reset
            cls.cstrided_copy = cfunc.double_vector_strided_copy
            cls.csafe_iget    = cfunc.double_vector_safe_iget
            cls.set_read_only = cfunc.double_vector_set_read_only
            cls.get_read_only = cfunc.double_vector_get_read_only
            cls.get_max       = cfunc.double_vector_get_max
            cls.get_min       = cfunc.double_vector_get_min
            cls.get_max_index = cfunc.double_vector_get_max_index
            cls.get_min_index = cfunc.double_vector_get_min_index
            cls.shift         = cfunc.double_vector_shift
            cls.scale         = cfunc.double_vector_scale
            cls.inplace_add   = cfunc.double_vector_inplace_add
            cls.inplace_mul   = cfunc.double_vector_inplace_mul
            cls.cassign       = cfunc.double_vector_assign
            cls.memcpy        = cfunc.double_vector_memcpy
            cls.cset_default  = cfunc.double_vector_set_default
            cls.cget_default  = cfunc.double_vector_get_default
            cls.alloc_data_copy = cfunc.double_vector_alloc_data_copy
            cls.get_element_size    = cfunc.double_vector_element_size
            cls.data_ptr            = cfunc.double_vector_data_ptr
            cls.numpy_dtype   = numpy.float64
            cls.def_fmt       = "%8.4f"
            cls.initialized = True

        obj = TVector.__new__( cls )
        return obj
    


class BoolVector(TVector):
    initialized = False

    def __new__( cls , *arglist ):
        if not cls.initialized:
            cls.csort         = cfunc.bool_vector_sort
            cls.crsort        = cfunc.bool_vector_rsort
            cls.alloc         = cfunc.bool_vector_alloc
            cls.alloc_copy    = cfunc.bool_vector_alloc_copy
            cls.free          = cfunc.bool_vector_free
            cls.get_size      = cfunc.bool_vector_size
            cls.iget          = cfunc.bool_vector_iget
            cls.iset          = cfunc.bool_vector_iset
            cls.fprintf       = cfunc.bool_vector_fprintf
            cls.cappend       = cfunc.bool_vector_append
            cls.idel_block    = cfunc.bool_vector_idel_block            
            cls.cclear        = cfunc.bool_vector_reset
            cls.cstrided_copy = cfunc.bool_vector_strided_copy
            cls.csafe_iget    = cfunc.bool_vector_safe_iget
            cls.set_read_only = cfunc.bool_vector_set_read_only
            cls.get_read_only = cfunc.bool_vector_get_read_only
            cls.get_max       = cfunc.bool_vector_get_max
            cls.get_min       = cfunc.bool_vector_get_min
            cls.get_max_index = cfunc.bool_vector_get_max_index
            cls.get_min_index = cfunc.bool_vector_get_min_index
            cls.shift         = cfunc.bool_vector_shift
            cls.scale         = cfunc.bool_vector_scale
            cls.inplace_add   = cfunc.bool_vector_inplace_add
            cls.inplace_mul   = cfunc.bool_vector_inplace_mul
            cls.cassign       = cfunc.bool_vector_assign
            cls.memcpy        = cfunc.bool_vector_memcpy
            cls.cset_default  = cfunc.bool_vector_set_default
            cls.cget_default  = cfunc.bool_vector_get_default
            cls.alloc_data_copy = cfunc.bool_vector_alloc_data_copy
            cls.get_element_size    = cfunc.bool_vector_element_size
            cls.data_ptr            = cfunc.bool_vector_data_ptr
            cls.numpy_dtype   = numpy.bool
            cls.def_fmt       = "%8d"
            cls.initialized = True

        obj = TVector.__new__( cls )
        return obj
    


class IntVector(TVector):
    initialized = False
    
    def __new__( cls , *arglist ):
        if not cls.initialized:
            cls.csort         = cfunc.int_vector_sort
            cls.crsort         = cfunc.int_vector_rsort
            cls.alloc         = cfunc.int_vector_alloc
            cls.alloc_copy    = cfunc.int_vector_alloc_copy
            cls.free          = cfunc.int_vector_free
            cls.get_size      = cfunc.int_vector_size
            cls.iget          = cfunc.int_vector_iget
            cls.iset          = cfunc.int_vector_iset
            cls.fprintf       = cfunc.int_vector_fprintf
            cls.cappend       = cfunc.int_vector_append
            cls.idel_block    = cfunc.int_vector_idel_block            
            cls.cclear        = cfunc.int_vector_reset
            cls.cstrided_copy = cfunc.int_vector_strided_copy
            cls.csafe_iget    = cfunc.int_vector_safe_iget
            cls.set_read_only = cfunc.int_vector_set_read_only
            cls.get_read_only = cfunc.int_vector_get_read_only
            cls.get_max       = cfunc.int_vector_get_max
            cls.get_min       = cfunc.int_vector_get_min
            cls.get_max_index = cfunc.int_vector_get_max_index
            cls.get_min_index = cfunc.int_vector_get_min_index
            cls.shift         = cfunc.int_vector_shift
            cls.scale         = cfunc.int_vector_scale
            cls.inplace_add   = cfunc.int_vector_inplace_add
            cls.inplace_mul   = cfunc.int_vector_inplace_mul
            cls.cassign        = cfunc.int_vector_assign
            cls.memcpy        = cfunc.int_vector_memcpy
            cls.cset_default  = cfunc.int_vector_set_default
            cls.cget_default  = cfunc.int_vector_get_default
            cls.alloc_data_copy = cfunc.int_vector_alloc_data_copy
            cls.data_ptr            = cfunc.int_vector_data_ptr
            cls.get_element_size    = cfunc.int_vector_element_size
            cls.numpy_dtype   = numpy.int32
            cls.def_fmt       = "%d"
            cls.initialized = True

        obj = TVector.__new__( cls )
        return obj
    
#################################################################

buffer_from_ptr = ctypes.pythonapi.PyBuffer_FromMemory
buffer_from_ptr.restype  = ctypes.py_object
buffer_from_ptr.argtypes = [ ctypes.c_void_p , ctypes.c_long ]

CWrapper.registerType( "double_vector" , DoubleVector )
CWrapper.registerType( "int_vector"    , IntVector )
CWrapper.registerType( "bool_vector"   , BoolVector ) 


cwrapper = CWrapper( libutil.lib )
cfunc    = CWrapperNameSpace("tvector")


cfunc.double_vector_alloc            = cwrapper.prototype("c_void_p   double_vector_alloc( int , double )")
cfunc.double_vector_alloc_copy       = cwrapper.prototype("c_void_p   double_vector_alloc_copy( double_vector )")
cfunc.double_vector_strided_copy     = cwrapper.prototype("c_void_p   double_vector_alloc_strided_copy( double_vector , int , int , int)")
cfunc.double_vector_free             = cwrapper.prototype("void   double_vector_free( double_vector )")
cfunc.double_vector_iget             = cwrapper.prototype("double double_vector_iget( double_vector , int )")
cfunc.double_vector_safe_iget        = cwrapper.prototype("double double_vector_safe_iget( int_vector , int )")
cfunc.double_vector_iset             = cwrapper.prototype("double double_vector_iset( double_vector , int , double)")
cfunc.double_vector_size             = cwrapper.prototype("int    double_vector_size( double_vector )")
cfunc.double_vector_append           = cwrapper.prototype("void   double_vector_append( double_vector , double )") 
cfunc.double_vector_idel_block       = cwrapper.prototype("void   double_vector_idel_block( double_vector , int , int )") 
cfunc.double_vector_fprintf          = cwrapper.prototype("void   double_vector_fprintf( double_vector , FILE , char* , char*)")
cfunc.double_vector_sort             = cwrapper.prototype("void   double_vector_sort( double_vector )") 
cfunc.double_vector_rsort            = cwrapper.prototype("void   double_vector_rsort( double_vector )") 
cfunc.double_vector_reset            = cwrapper.prototype("void   double_vector_reset( double_vector )") 
cfunc.double_vector_get_read_only    = cwrapper.prototype("bool   double_vector_set_read_only( double_vector )") 
cfunc.double_vector_set_read_only    = cwrapper.prototype("void   double_vector_set_read_only( double_vector , bool )") 
cfunc.double_vector_get_max          = cwrapper.prototype("double    double_vector_get_max( double_vector )")
cfunc.double_vector_get_min          = cwrapper.prototype("double    double_vector_get_min( double_vector )")
cfunc.double_vector_get_max_index    = cwrapper.prototype("int    double_vector_get_max_index( double_vector , bool)")
cfunc.double_vector_get_min_index    = cwrapper.prototype("int    double_vector_get_min_index( double_vector , bool)")
cfunc.double_vector_shift            = cwrapper.prototype("void   double_vector_shift( double_vector , double )")
cfunc.double_vector_scale            = cwrapper.prototype("void   double_vector_scale( double_vector , double )") 
cfunc.double_vector_inplace_add      = cwrapper.prototype("void   double_vector_inplace_add( double_vector , double_vector )")
cfunc.double_vector_inplace_mul      = cwrapper.prototype("void   double_vector_inplace_mul( double_vector , double_vector )")
cfunc.double_vector_assign              = cwrapper.prototype("void   double_vector_set_all( double_vector , double)")  
cfunc.double_vector_memcpy              = cwrapper.prototype("void   double_vector_memcpy(double_vector , double_vector )")
cfunc.double_vector_set_default         = cwrapper.prototype("void   double_vector_set_default( double_vector , double)")
cfunc.double_vector_get_default         = cwrapper.prototype("double    double_vector_get_default( double_vector )")
cfunc.double_vector_alloc_data_copy     = cwrapper.prototype("double*  double_vector_alloc_data_copy( double_vector )")
cfunc.double_vector_data_ptr            = cwrapper.prototype("double*  double_vector_get_ptr( double_vector )")
cfunc.double_vector_element_size        = cwrapper.prototype("int      double_vector_element_size( double_vector )")


cfunc.int_vector_alloc_copy          = cwrapper.prototype("c_void_p int_vector_alloc_copy( int_vector )")
cfunc.int_vector_alloc               = cwrapper.prototype("c_void_p   int_vector_alloc( int , int )")
cfunc.int_vector_strided_copy        = cwrapper.prototype("c_void_p   int_vector_alloc_strided_copy( int_vector , int , int , int)")
cfunc.int_vector_free                = cwrapper.prototype("void   int_vector_free( int_vector )")
cfunc.int_vector_iget                = cwrapper.prototype("int    int_vector_iget( int_vector , int )")
cfunc.int_vector_safe_iget           = cwrapper.prototype("int    int_vector_safe_iget( int_vector , int )")
cfunc.int_vector_iset                = cwrapper.prototype("int    int_vector_iset( int_vector , int , int)")
cfunc.int_vector_size                = cwrapper.prototype("int    int_vector_size( int_vector )")
cfunc.int_vector_append              = cwrapper.prototype("void   int_vector_append( int_vector , int )") 
cfunc.int_vector_idel_block          = cwrapper.prototype("void   int_vector_idel_block( int_vector , int , int )") 
cfunc.int_vector_fprintf             = cwrapper.prototype("void   int_vector_fprintf( int_vector , FILE , char* , char*)")
cfunc.int_vector_sort                = cwrapper.prototype("void   int_vector_sort( int_vector )") 
cfunc.int_vector_rsort               = cwrapper.prototype("void   int_vector_rsort( int_vector )") 
cfunc.int_vector_reset               = cwrapper.prototype("void   int_vector_reset( int_vector )") 
cfunc.int_vector_set_read_only       = cwrapper.prototype("void   int_vector_set_read_only( int_vector , bool )") 
cfunc.int_vector_get_read_only       = cwrapper.prototype("bool   int_vector_get_read_only( int_vector )") 
cfunc.int_vector_get_max             = cwrapper.prototype("int    int_vector_get_max( int_vector )")
cfunc.int_vector_get_min             = cwrapper.prototype("int    int_vector_get_min( int_vector )")
cfunc.int_vector_get_max_index       = cwrapper.prototype("int    int_vector_get_max_index( int_vector , bool)")
cfunc.int_vector_get_min_index       = cwrapper.prototype("int    int_vector_get_min_index( int_vector , bool)")
cfunc.int_vector_shift               = cwrapper.prototype("void   int_vector_shift( int_vector , int )")
cfunc.int_vector_scale               = cwrapper.prototype("void   int_vector_scale( int_vector , int )") 
cfunc.int_vector_inplace_add         = cwrapper.prototype("void   int_vector_inplace_add( int_vector , int_vector )")
cfunc.int_vector_inplace_mul         = cwrapper.prototype("void   int_vector_inplace_mul( int_vector , int_vector )")
cfunc.int_vector_assign              = cwrapper.prototype("void   int_vector_set_all( int_vector , int)")  
cfunc.int_vector_memcpy              = cwrapper.prototype("void   int_vector_memcpy(int_vector , int_vector )")
cfunc.int_vector_set_default         = cwrapper.prototype("void   int_vector_set_default( int_vector , int)")
cfunc.int_vector_get_default         = cwrapper.prototype("int    int_vector_get_default( int_vector )")
cfunc.int_vector_alloc_data_copy     = cwrapper.prototype("int*  int_vector_alloc_data_copy( int_vector )")
cfunc.int_vector_data_ptr            = cwrapper.prototype("int*  int_vector_get_ptr( int_vector )")
cfunc.int_vector_element_size        = cwrapper.prototype("int    int_vector_element_size( int_vector )")


cfunc.bool_vector_alloc_copy          = cwrapper.prototype("c_void_p bool_vector_alloc_copy( bool_vector )")
cfunc.bool_vector_alloc               = cwrapper.prototype("c_void_p   bool_vector_alloc( bool , bool )")
cfunc.bool_vector_strided_copy        = cwrapper.prototype("c_void_p   bool_vector_alloc_strided_copy( bool_vector , bool , bool , bool)")
cfunc.bool_vector_free                = cwrapper.prototype("void   bool_vector_free( bool_vector )")
cfunc.bool_vector_iget                = cwrapper.prototype("bool    bool_vector_iget( bool_vector , bool )")
cfunc.bool_vector_safe_iget           = cwrapper.prototype("bool    bool_vector_safe_iget( bool_vector , bool )")
cfunc.bool_vector_iset                = cwrapper.prototype("bool    bool_vector_iset( bool_vector , bool , bool)")
cfunc.bool_vector_size                = cwrapper.prototype("bool    bool_vector_size( bool_vector )")
cfunc.bool_vector_append              = cwrapper.prototype("void   bool_vector_append( bool_vector , bool )") 
cfunc.bool_vector_idel_block          = cwrapper.prototype("void   bool_vector_idel_block( bool_vector , bool , bool )") 
cfunc.bool_vector_fprintf             = cwrapper.prototype("void   bool_vector_fprintf( bool_vector , FILE , char* , char*)")
cfunc.bool_vector_sort                = cwrapper.prototype("void   bool_vector_sort( bool_vector )") 
cfunc.bool_vector_rsort               = cwrapper.prototype("void   bool_vector_rsort( bool_vector )") 
cfunc.bool_vector_reset               = cwrapper.prototype("void   bool_vector_reset( bool_vector )") 
cfunc.bool_vector_set_read_only       = cwrapper.prototype("void   bool_vector_set_read_only( bool_vector , bool )") 
cfunc.bool_vector_get_read_only       = cwrapper.prototype("bool   bool_vector_get_read_only( bool_vector )") 
cfunc.bool_vector_get_max             = cwrapper.prototype("bool    bool_vector_get_max( bool_vector )")
cfunc.bool_vector_get_min             = cwrapper.prototype("bool    bool_vector_get_min( bool_vector )")
cfunc.bool_vector_get_max_index       = cwrapper.prototype("bool    bool_vector_get_max_index( bool_vector , bool)")
cfunc.bool_vector_get_min_index       = cwrapper.prototype("bool    bool_vector_get_min_index( bool_vector , bool)")
cfunc.bool_vector_shift               = cwrapper.prototype("void   bool_vector_shift( bool_vector , bool )")
cfunc.bool_vector_scale               = cwrapper.prototype("void   bool_vector_scale( bool_vector , bool )") 
cfunc.bool_vector_inplace_add         = cwrapper.prototype("void   bool_vector_inplace_add( bool_vector , bool_vector )")
cfunc.bool_vector_inplace_mul         = cwrapper.prototype("void   bool_vector_inplace_mul( bool_vector , bool_vector )")
cfunc.bool_vector_assign              = cwrapper.prototype("void   bool_vector_set_all( bool_vector , bool)")  
cfunc.bool_vector_memcpy              = cwrapper.prototype("void   bool_vector_memcpy(bool_vector , bool_vector )")
cfunc.bool_vector_set_default         = cwrapper.prototype("void   bool_vector_set_default( bool_vector , bool)")
cfunc.bool_vector_get_default         = cwrapper.prototype("bool   bool_vector_get_default( bool_vector )")
cfunc.bool_vector_alloc_data_copy     = cwrapper.prototype("bool*  bool_vector_alloc_data_copy( bool_vector )")
cfunc.bool_vector_data_ptr            = cwrapper.prototype("bool*  bool_vector_get_ptr( bool_vector )")
cfunc.bool_vector_element_size        = cwrapper.prototype("int    bool_vector_element_size( bool_vector )")

#  Copyright (C) 2016  Statoil ASA, Norway. 
#   
#  The file 'surface' is part of ERT - Ensemble based Reservoir Tool. 
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
Create a polygon
"""
import os.path

from ert.cwrap import BaseCClass
from ert.geo import GeoPrototype


class Surface(BaseCClass):
    TYPE_NAME = "surface"

    _alloc        = GeoPrototype("void*  geo_surface_fload_alloc_irap( char* , bool )" , bind = False)
    _free         = GeoPrototype("void   geo_surface_free( surface )")
    _get_nx       = GeoPrototype("int    geo_surface_get_nx( surface )")
    _get_ny       = GeoPrototype("int    geo_surface_get_ny( surface )")
    _iget_zvalue  = GeoPrototype("double geo_surface_iget_zvalue( surface , int)")
    _iset_zvalue  = GeoPrototype("void   geo_surface_iset_zvalue( surface , int , double)")
    _write        = GeoPrototype("void   geo_surface_fprintf_irap( surface , char* )")
    _equal        = GeoPrototype("bool   geo_surface_equal( surface , surface )")
    _header_equal = GeoPrototype("bool   geo_surface_equal_header( surface , surface )")
    _copy         = GeoPrototype("surface_obj geo_surface_alloc_copy( surface , bool )")
    _assign       = GeoPrototype("void   geo_surface_assign_value( surface , double )")
    _scale        = GeoPrototype("void   geo_surface_scale( surface , double )")
    _shift        = GeoPrototype("void   geo_surface_shift( surface , double )")
    _iadd         = GeoPrototype("void   geo_surface_iadd( surface , surface )")
    _imul         = GeoPrototype("void   geo_surface_imul( surface , surface )")
    _isub         = GeoPrototype("void   geo_surface_isub( surface , surface )")
    _isqrt        = GeoPrototype("void   geo_surface_isqrt( surface )")

    
    def __init__(self, filename):
        """
        This will load a irap surface from file. The surface should
        consist of a header and a set z values.
        """
        if os.path.isfile( filename ):
            c_ptr = self._alloc(filename , True)
            super(Surface , self).__init__(c_ptr)
        else:
            raise IOError("No such file: %s" % filename)

    
    def __eq__(self , other):
        """
        Compares two Surface instances, both header and data must be equal
        to compare as equal.
        """
        if isinstance( other , Surface):
            return self._equal(other)
        else:
            return False


    def headerEqual(self , other):
        return self._header_equal( other)
        

    def __iadd__(self , other):
        if isinstance(other , Surface):
            if self.headerEqual(other):
                self._iadd(other)
            else:
                raise ValueError("Tried to add incompatible surfaces")
        else:
            self._shift(other)
        return self


    def __isub__(self , other):
        if isinstance(other , Surface):
            if self.headerEqual(other):
                self._isub(other)
            else:
                raise ValueError("Tried to subtract incompatible surfaces")
        else:
            self._shift( -other)
        return self


    def __imul__(self , other):
        if isinstance(other , Surface):
            if self.headerEqual(other):
                self._imul( other)
            else:
                raise ValueError("Tried to add multiply ncompatible surfaces")
        else:
            self._scale( other)
        return self


    def __idiv__(self , other):
        self._scale( 1.0/other)
        return self


    def __add__(self , other):
        copy = self.copy()
        copy += other
        return copy


    def __mul__(self , other):
        copy = self.copy()
        copy *= other
        return copy


    def __sub__(self , other):
        copy = self.copy()
        copy -= other
        return copy

    
    def __div__(self , other):
        copy = self.copy()
        copy /= other
        return copy

    
    def __len__(self):
        """
        The number of values in the surface.
        """
        return self.getNX() * self.getNY() 


    def inplaceSqrt(self):
        """
        Will do an inplcae sqrt opearation.
        """
        self._isqrt( )
        return self

    

    def sqrt(self):
        """
        Will return a new surface where all elements have been sqrt{ .. }.
        """
        copy = self.copy( )
        copy.inplaceSqrt( )
        return copy

    
    def copy(self , copy_data = True):
        """Will create a deep copy of self, if copy_data is set to False the
        copy will have all z-values set to zero.
        """
        return self._copy( copy_data)
        

    def write(self , filename):

        """
        Will write the surface as an ascii formatted file to @filename.
        """
        self._write(  filename )

    

    def assign(self , value):
        """
        Will set all the values in the surface to @value"
        """
        self._assign(value)


    def __setitem__(self , index , value):
        if isinstance(index , int):
            if index >= len(self):
                raise IndexError("Invalid index:%d - valid range [0,%d)" % (index , len(self)))
            if index < 0:
                index += len(self)

            self._iset_zvalue(index , value)
        else:
             raise TypeError("Invalid index type:%s - must be integer" % index)

         

    def __getitem__(self , index):
        if isinstance(index , int):
            if index >= len(self):
                raise IndexError("Invalid index:%d - valid range [0,%d)" % (index , len(self)))
            if index < 0:
                index += len(self)

            return self._iget_zvalue( index)
        else:
             raise TypeError("Invalid index type:%s - must be integer" % index)

    def getNX(self):
        return self._get_nx(  )


    def getNY(self):
        return self._get_ny(  )
        

    def free(self):
        self._free( )

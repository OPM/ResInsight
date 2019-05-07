#  Copyright (C) 2016  Equinor ASA, Norway.
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
from __future__ import division

"""
Create a polygon
"""
import os.path
import ctypes

from cwrap import BaseCClass
from ecl import EclPrototype
from ecl.util.geometry import GeoPointset


class Surface(BaseCClass):
    TYPE_NAME = "surface"

    _alloc        = EclPrototype("void*  geo_surface_fload_alloc_irap( char* , bool )" , bind = False)
    _free         = EclPrototype("void   geo_surface_free( surface )")
    _new          = EclPrototype("void*  geo_surface_alloc_new( int, int, double, double, double, double, double )", bind = False)
    _get_nx       = EclPrototype("int    geo_surface_get_nx( surface )")
    _get_ny       = EclPrototype("int    geo_surface_get_ny( surface )")
    _iget_zvalue  = EclPrototype("double geo_surface_iget_zvalue( surface , int)")
    _iset_zvalue  = EclPrototype("void   geo_surface_iset_zvalue( surface , int , double)")
    _write        = EclPrototype("void   geo_surface_fprintf_irap( surface , char* )")
    _equal        = EclPrototype("bool   geo_surface_equal( surface , surface )")
    _header_equal = EclPrototype("bool   geo_surface_equal_header( surface , surface )")
    _copy         = EclPrototype("surface_obj geo_surface_alloc_copy( surface , bool )")
    _assign       = EclPrototype("void   geo_surface_assign_value( surface , double )")
    _scale        = EclPrototype("void   geo_surface_scale( surface , double )")
    _shift        = EclPrototype("void   geo_surface_shift( surface , double )")
    _iadd         = EclPrototype("void   geo_surface_iadd( surface , surface )")
    _imul         = EclPrototype("void   geo_surface_imul( surface , surface )")
    _isub         = EclPrototype("void   geo_surface_isub( surface , surface )")
    _isqrt        = EclPrototype("void   geo_surface_isqrt( surface )")
    _iget_xy      = EclPrototype("void   geo_surface_iget_xy(surface, int, double*, double*)")
    _get_pointset = EclPrototype("geo_pointset_ref geo_surface_get_pointset(surface)")


    def __init__(self, filename=None, nx=None, ny=None, xinc=None, yinc=None,
                                      xstart=None, ystart=None, angle=None):
        """
        This will load a irap surface from file. The surface should
        consist of a header and a set z values.
        """
        if filename is not None:
            filename = str(filename)
            if os.path.isfile( filename ):
                c_ptr = self._alloc(filename , True)
                super(Surface , self).__init__(c_ptr)
            else:
                raise IOError('No such file "%s".' % filename)
        else:
            s_args = [nx, ny, xinc, yinc, xstart, ystart, angle]
            if None in s_args:
                raise ValueError('Missing argument for creating surface, all values must be set, was: %s' % str(s_args))
            c_ptr = self._new(*s_args)
            super(Surface , self).__init__(c_ptr)

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

    def __itruediv__(self , other):
        self._scale(1.0 / other)
        return self

    def __idiv__(self, other):
        return self.__itruediv__(other)


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


    def __truediv__(self , other):
        copy = self.copy()
        copy /= other
        return copy


    def __div__(self, other):
        return self.__truediv__(other)


    def __len__(self):
        """
        The number of values in the surface.
        """
        return self.getNX() * self.getNY()


    def inplaceSqrt(self):
        """
        Will do an inplace sqrt operation.
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
            idx = index
            ls = len(self)
            if idx < 0:
                idx += ls
            if 0 <= idx < ls:
                return self._iget_zvalue(idx)
            else:
                raise IndexError("Invalid index:%d - valid range [0,%d)" % (index , len(self)))
        else:
            raise TypeError("Invalid index type:%s - must be integer" % index)


    def getXY(self, index):
        """Gets the index'th (x,y) coordinate"""
        if isinstance(index, int):
            idx = index
            if idx < 0:
                idx += len(self)
            if not 0 <= idx < len(self):
                raise IndexError("Invalid index:%d - valid range [0,%d)" % (index, len(self)))
            index = idx
        else:
            raise TypeError("Invalid index type:%s - must be integer" % index)

        x = ctypes.c_double()
        y = ctypes.c_double()
        self._iget_xy(index, ctypes.byref(x), ctypes.byref(y))

        return x.value, y.value


    def getNX(self):
        return self._get_nx()


    def getNY(self):
        return self._get_ny()

    def getPointset(self):
        return self._get_pointset()

    def _assert_idx_or_i_and_j(self, idx, i, j):
        if idx is None:
            if i is None or j is None:
                raise ValueError('idx is None, i and j must be ints, was %s and %s.' % (i, j))
        else:
            if i is not None or j is not None:
                raise ValueError('idx is set, i and j must be None, was %s and %s.' % (i, j))


    def getXYZ(self, idx=None, i=None, j=None):
        """Returns a tuple of 3 floats, (x,y,z) for given global index, or i and j."""
        self._assert_idx_or_i_and_j(idx, i, j)
        if idx is None:
            nx, ny = self.getNX(), self.getNY()
            i_idx, j_idx = i,j
            if i_idx < 0:
                i_idx += self.getNX()
            if j_idx < 0:
                j_idx += self.getNY()
            if 0 <= i_idx < self.getNX() and 0 <= j_idx < self.getNY():
                idx = j_idx * self.getNX() + i_idx
            else:
                fmt = 'Index error: i=%d not in [0,nx=%d) or j=%d not in [0,ny=%d).'
                raise IndexError(fmt % (i, nx, j, ny))
        x,y = self.getXY(idx)
        z = self[idx]
        return (x,y,z)


    def free(self):
        self._free()

    def __repr__(self):
        cnt = 'nx=%d, ny=%d' % (self.getNX(), self.getNY())
        return self._create_repr(cnt)

#  Copyright (C) 2017  Equinor ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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
from cwrap import BaseCClass
from ecl import EclPrototype

class GeoPointset(BaseCClass):
    TYPE_NAME = "geo_pointset"

    _alloc      = EclPrototype("void*   geo_pointset_alloc(bool)", bind=False)
    _free       = EclPrototype("void    geo_pointset_free(geo_pointset)")
    #_add_xyz    = EclPrototype("void    geo_pointset_add_xyz(geo_pointset, double, double, double)")
    _get_size   = EclPrototype("int     geo_pointset_get_size(geo_pointset)")
    #_iget_xy    = EclPrototype("void    geo_pointset_iget_xy(geo_pointset, int, double*, double*)")
    #_get_zcoord = EclPrototype("double* geo_pointset_get_zcoord(geo_pointset)")
    _equal      = EclPrototype("bool    geo_pointset_equal(geo_pointset, geo_pointset)")
    _iget_z     = EclPrototype("double  geo_pointset_iget_z(geo_pointset, int)")
    #_iset_z     = EclPrototype("void    geo_pointset_iset_z(geo_pointset, int, double)")
    #_memcpy     = EclPrototype("void    geo_pointset_memcpy(geo_pointset, geo_pointset, bool)")
    #_shift_z    = EclPrototype("void    geo_pointset_shift_z(geo_pointset, double)")
    #_assign_z   = EclPrototype("void    geo_pointset_assign_z(geo_pointset, double)")
    #_scale_z    = EclPrototype("void    geo_pointset_scale_z(geo_pointset, double)")
    #_imul       = EclPrototype("void    geo_pointset_imul(geo_pointset, geo_pointset)")
    #_iadd       = EclPrototype("void    geo_pointset_iadd(geo_pointset, geo_pointset)")
    #_isub       = EclPrototype("void    geo_pointset_isub(geo_pointset, geo_pointset)")
    #_isqrt      = EclPrototype("void    geo_pointset_isqrt(geo_pointset)")


    def __init__(self, external_z=False):
        c_ptr = self._alloc(external_z)
        if c_ptr:
            super(GeoPointset, self).__init__(c_ptr)
        else:
            ext = 'external' if external_z else 'internal'
            raise ValueError('Failed to construct GeoPointset with %s_z.' % ext)

    @staticmethod
    def fromSurface(surface):
        return surface.getPointset()

    def __eq__(self, other):
        if isinstance(other, GeoPointset):
            return self._equal(other)
        return NotImplemented

    def __getitem__(self, key):
        size = len(self)
        if isinstance(key, int):
            idx = key
            if idx < 0:
                idx += size
            if 0 <= idx < size:
                return self._iget_z(idx)
            else:
                raise IndexError('Invalid index, must be in [0, %d), was: %d.' % (size, key))
        else:
            # TODO implement slicing?
            raise ValueError('Index must be int, not %s.' % type(key))

    def __len__(self):
        return self._get_size()

    def __repr__(self):
        return self._create_repr('len=%d' % len(self))

    def free(self):
        self._free()

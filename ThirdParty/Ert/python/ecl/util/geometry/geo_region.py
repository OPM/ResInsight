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
from ecl.util.util import IntVector
from ecl import EclPrototype
from .cpolyline import CPolyline
from ctypes import c_double

cpair = c_double * 2  # this is a function that maps two doubles to a double*

class GeoRegion(BaseCClass):
    TYPE_NAME = "geo_region"

    _alloc = EclPrototype("void* geo_region_alloc(geo_pointset, bool)", bind=False)
    _free  = EclPrototype("void  geo_region_free(geo_region)")
    _reset = EclPrototype("void  geo_region_reset(geo_region)")
    _get_index_list = EclPrototype("int_vector_ref geo_region_get_index_list(geo_region)")
    _select_inside_polygon    = EclPrototype("void geo_region_select_inside_polygon(geo_region, geo_polygon)")
    _select_outside_polygon   = EclPrototype("void geo_region_select_outside_polygon(geo_region, geo_polygon)")
    _deselect_inside_polygon  = EclPrototype("void geo_region_deselect_inside_polygon(geo_region, geo_polygon)")
    _deselect_outside_polygon = EclPrototype("void geo_region_deselect_outside_polygon(geo_region, geo_polygon)")
    _select_above_line        = EclPrototype("void geo_region_select_above_line(geo_region, double*, double*)")
    _select_below_line        = EclPrototype("void geo_region_select_below_line(geo_region, double*, double*)")
    _deselect_above_line      = EclPrototype("void geo_region_deselect_above_line(geo_region, double*, double*)")
    _deselect_below_line      = EclPrototype("void geo_region_deselect_below_line(geo_region, double*, double*)")


    def __init__(self, pointset, preselect=False):
        self._preselect = True if preselect else False
        c_ptr = self._alloc(pointset, self._preselect)
        if c_ptr:
            super(GeoRegion, self).__init__(c_ptr)
        else:
            raise ValueError('Could not construct GeoRegion from pointset %s.' % pointset)


    def getActiveList(self):
        return self._get_index_list()

    def _assert_polygon(self, polygon):
        if not isinstance(polygon, CPolyline):
            raise ValueError('Need to select with a CPolyline, not %s.'
                             % type(polygon))


    def _construct_cline(self, line):
        """Takes a line ((x1,y1), (x2,y2)) and returns two double[2]* but
        reordered to (x1x2, y1y2).
        """
        try:
            p1, p2 = line
            x1, y1 = map(float, p1)
            x2, y2 = map(float, p2)
        except Exception as err:
            err_msg = 'Select with pair ((x1,y1), (x2,y2)), not %s (%s).'
            raise ValueError(err_msg % (line, err))
        x1x2_ptr = cpair(x1, x2)
        y1y2_ptr = cpair(y1, y2)
        return x1x2_ptr, y1y2_ptr


    def select_inside(self, polygon):
        self._assert_polygon(polygon)
        self._select_inside_polygon(polygon)

    def select_outside(self, polygon):
        self._assert_polygon(polygon)
        self._select_outside_polygon(polygon)

    def deselect_inside(self, polygon):
        self._assert_polygon(polygon)
        self._deselect_inside_polygon(polygon)

    def deselect_outside(self, polygon):
        self._assert_polygon(polygon)
        self._deselect_outside_polygon(polygon)


    def select_above(self, line):
        x_ptr, y_ptr = self._construct_cline(line)
        self._select_above_line(x_ptr, y_ptr)

    def select_below(self, line):
        x_ptr, y_ptr = self._construct_cline(line)
        self._select_below_line(x_ptr, y_ptr)

    def deselect_above(self, line):
        x_ptr, y_ptr = self._construct_cline(line)
        self._deselect_above_line(x_ptr, y_ptr)

    def deselect_below(self, line):
        x_ptr, y_ptr = self._construct_cline(line)
        self._deselect_below_line(x_ptr, y_ptr)


    def __len__(self):
        """Returns the size of the active list, not the size of the
        underlying pointset"""
        return len(self._get_index_list())

    def __repr__(self):
        ls = len(self)
        il = repr(self.getActiveList())
        pres = 'preselected' if self._preselect else 'not preselected'
        return self._create_repr('size=%d, active_list=<%s>, %s' % (ls, il, pres))

    def free(self):
        self._free()

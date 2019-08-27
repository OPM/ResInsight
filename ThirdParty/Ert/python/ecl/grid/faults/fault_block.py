#  Copyright (C) 2014  Equinor ASA, Norway.
#
#  The file 'fault_block.py' is part of ERT - Ensemble based Reservoir Tool.
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

import ctypes
from cwrap import BaseCClass

from ecl.util.util import monkey_the_camel
from ecl.util.util import DoubleVector, IntVector
from ecl import EclPrototype
from ecl.util.geometry import Polyline, GeometryTools, CPolylineCollection

class FaultBlockCell(object):

    def __init__(self, i, j, k, x, y, z):
        self.i = i
        self.j = j
        self.k = k

        self.x = x
        self.y = y
        self.z = z


    def __str__(self):
        return "(%d,%d)" % (self.i, self.j)



class FaultBlock(BaseCClass):
    TYPE_NAME = "fault_block"

    _get_xc                = EclPrototype("double         fault_block_get_xc(fault_block)")
    _get_yc                = EclPrototype("double         fault_block_get_yc(fault_block)")
    _get_block_id          = EclPrototype("int            fault_block_get_id(fault_block)")
    _get_size              = EclPrototype("int            fault_block_get_size(fault_block)")
    _export_cell           = EclPrototype("void           fault_block_export_cell(fault_block, int, int*, int*, int*, double*, double*, double*)")
    _assign_to_region      = EclPrototype("void           fault_block_assign_to_region(fault_block, int)")
    _get_region_list       = EclPrototype("int_vector_ref fault_block_get_region_list(fault_block)")
    _add_cell              = EclPrototype("void           fault_block_add_cell(fault_block,  int, int)")
    _get_global_index_list = EclPrototype("int_vector_ref fault_block_get_global_index_list(fault_block)")
    _trace_edge            = EclPrototype("void           fault_block_trace_edge(fault_block, double_vector, double_vector, int_vector)")
    _get_neighbours        = EclPrototype("void           fault_block_list_neighbours(fault_block, bool, geo_polygon_collection, int_vector)")
    _free                  = EclPrototype("void           fault_block_free__(fault_block)")


    def __init__(self, *args, **kwargs):
        raise NotImplementedError("Class can not be instantiated directly!")


    def __getitem__(self, index):
        if isinstance(index, int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                x = ctypes.c_double()
                y = ctypes.c_double()
                z = ctypes.c_double()

                i = ctypes.c_int()
                j = ctypes.c_int()
                k = ctypes.c_int()

                self._export_cell(index,
                                  ctypes.byref(i), ctypes.byref(j), ctypes.byref(k),
                                  ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
                return FaultBlockCell(i.value, j.value, k.value, x.value, y.value, z.value)
            else:
                raise IndexError("Index:%d out of range: [0,%d)" % (index, len(self)))
        else:
            raise TypeError("Index:%s wrong type - integer expected")

    def __str__(self):
        return "Block ID: %d" % self.getBlockID()


    def __len__(self):
        return self._get_size()

    def free(self):
        self._free()

    def get_centroid(self):
        xc = self._get_xc()
        yc = self._get_yc()
        return (xc,yc)


    def count_inside(self, polygon):
        """
        Will count the number of points in block which are inside polygon.
        """
        inside = 0
        for p in self:
            if GeometryTools.pointInPolygon((p.x, p.y), polygon):
                inside += 1

        return inside


    def get_block_id(self):
        return self._get_block_id()


    def assign_to_region(self, region_id):
        self._assign_to_region(region_id)


    def get_region_list(self):
        regionList = self._get_region_list()
        return regionList.copy()

    def add_cell(self, i, j):
        self._add_cell(i, j)

    def get_global_index_list(self):
        return self._get_global_index_list()


    def get_edge_polygon(self):
        x_list = DoubleVector()
        y_list = DoubleVector()
        cell_list = IntVector()

        self._trace_edge(x_list, y_list, cell_list)
        p = Polyline()
        for (x,y) in zip(x_list, y_list):
            p.addPoint(x,y)
        return p


    def contains_polyline(self, polyline):
        """
        Will return true if at least one point from the polyline is inside the block.
        """
        edge_polyline = self.getEdgePolygon()
        for p in polyline:
            if GeometryTools.pointInPolygon(p, edge_polyline):
                return True
        else:
            edge_polyline.assertClosed()
            return GeometryTools.polylinesIntersect(edge_polyline, polyline)


    def get_neighbours(self, polylines=None, connected_only=True):
        """
        Will return a list of FaultBlock instances which are in direct
        contact with this block.
        """
        neighbour_id_list = IntVector()
        if polylines is None:
            polylines = CPolylineCollection()

        self._get_neighbours(connected_only, polylines, neighbour_id_list)

        parent_layer = self.getParentLayer()
        neighbour_list = []
        for id in neighbour_id_list:
            neighbour_list.append(parent_layer.getBlock(id))
        return neighbour_list


    def get_parent_layer(self):
        return self.parent()


monkey_the_camel(FaultBlock, 'getCentroid', FaultBlock.get_centroid)
monkey_the_camel(FaultBlock, 'countInside', FaultBlock.count_inside)
monkey_the_camel(FaultBlock, 'getBlockID', FaultBlock.get_block_id)
monkey_the_camel(FaultBlock, 'assignToRegion', FaultBlock.assign_to_region)
monkey_the_camel(FaultBlock, 'getRegionList', FaultBlock.get_region_list)
monkey_the_camel(FaultBlock, 'addCell', FaultBlock.add_cell)
monkey_the_camel(FaultBlock, 'getGlobalIndexList', FaultBlock.get_global_index_list)
monkey_the_camel(FaultBlock, 'getEdgePolygon', FaultBlock.get_edge_polygon)
monkey_the_camel(FaultBlock, 'containsPolyline', FaultBlock.contains_polyline)
monkey_the_camel(FaultBlock, 'getNeighbours', FaultBlock.get_neighbours)
monkey_the_camel(FaultBlock, 'getParentLayer', FaultBlock.get_parent_layer)

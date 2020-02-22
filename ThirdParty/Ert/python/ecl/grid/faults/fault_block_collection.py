#  Copyright (C) 2014  Equinor ASA, Norway.
#
#  The file 'fault_block_collection.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ecl.util.util import monkey_the_camel
from ecl import EclPrototype


class FaultBlockCollection(BaseCClass):
    TYPE_NAME = "fault_block_collection"
    _alloc         = EclPrototype("void* fault_block_collection_alloc(ecl_grid)", bind=False)
    _free          = EclPrototype("void  fault_block_collection_free(fault_block_collection)")
    _num_layers    = EclPrototype("int   fault_block_collection_num_layers(fault_block_collection)")
    _scan_keyword  = EclPrototype("bool  fault_block_collection_scan_kw(fault_block_collection, ecl_kw)")
    _get_layer     = EclPrototype("fault_block_layer_ref  fault_block_collection_get_layer(fault_block_collection, int)")

    def __init__(self, grid):
        c_ptr = self._alloc(grid)
        if c_ptr:
            super(FaultBlockCollection, self).__init__(c_ptr)
        else:
            raise ValueError("Invalid input - failed to create FaultBlockCollection")

        # The underlying C implementation uses lazy evaluation and
        # needs to hold on to the grid reference. We therefore take
        # references to it here, to protect against premature garbage
        # collection.
        self.grid_ref = grid


    def __len__(self):
        return self._num_layers()


    def __repr__(self):
        return self._create_repr('len=%s' % len(self))


    def __getitem__(self, index):
        """
        @rtype: FaultBlockLayer
        """
        if isinstance(index, int):
            if 0 <= index < len(self):
                return self._get_layer(index).setParent(self)
            else:
                raise IndexError("Index:%d out of range [0,%d)" % (index, len(self)))
        else:
            raise TypeError("Index should be integer type")


    def get_layer(self, k):
        """
        @rtype: FaultBlockLayer
        """
        return self[k]

    def free(self):
        self._free()


    def scan_keyword(self, fault_block_kw):
        ok = self._scan_keyword(fault_block_kw)
        if not ok:
            raise ValueError("The fault block keyword had wrong type/size")


monkey_the_camel(FaultBlockCollection, 'getLayer', FaultBlockCollection.get_layer)
monkey_the_camel(FaultBlockCollection, 'scanKeyword', FaultBlockCollection.scan_keyword)

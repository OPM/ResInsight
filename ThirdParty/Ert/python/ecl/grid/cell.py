#!/usr/bin/env python
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

class Cell(object):

    def __init__(self, grid, global_index):
        self._grid = grid
        self._idx = global_index
        self._ijk = grid.get_ijk(global_index=self._idx)
        self._aidx = grid.get_active_index(global_index=self._idx)

    @property
    def volume(self):
        return self._grid.cell_volume(global_index=self._idx)

    @property
    def global_index(self):
        return self._idx

    @property
    def active_index(self):
        return self._aidx

    @property
    def ijk(self):
        return self._ijk
    @property
    def i(self):
        return self._ijk[0]
    @property
    def j(self):
        return self._ijk[1]
    @property
    def k(self):
        return self._ijk[2]

    @property
    def active(self):
        return self._aidx >= 0

    def eval(self, kw):
        return self._grid.grid_value(kw, self.i, self.j, self.k)

    @property
    def fracture(self):
        return self._grid.active_fracture_index(global_index=self._idx)

    @property
    def dz(self):
        return self._grid.cell_dz(global_index=self._idx)

    @property
    def dimension(self):
        return self._grid.get_cell_dims(global_index=self._idx)

    @property
    def valid_geometry(self):
        return self._grid.valid_cell_geometry(global_index=self._idx)

    @property
    def valid(self):
        return not self._grid.cell_invalid(global_index=self._idx)

    def __contains__(self, coord):
        """
        Will check if this cell contains point given by world
        coordinates (x,y,z)=coord.
        """
        if len(coord) != 3:
            raise ValueError('Cell contains takes a triple (x,y,z), was given %s.' % coord)
        x,y,z = coord
        return self._grid._cell_contains(self._idx, x,y,z)

    def __eq__(self, other):
        if isinstance(other, Cell):
            idx_eq = self.global_index == other.global_index
            return idx_eq and self._grid == other._grid
        return NotImplemented

    def __neq__(self, other):
        if isinstance(other, Cell):
            return not self == other
        return NotImplemented

    def hash(self):
        return hash((self._idx, self._aidx, self.ijk))

    @property
    def coordinate(self):
        return self._grid.get_xyz(global_index=self._idx)

    @property
    def corners(self):
        """
        Return xyz for each of the eight vertices, indexed by:

        lower layer:   upper layer

         2---3           6---7
         |   |           |   |
         0---1           4---5
        """
        cs = lambda c : self._grid.get_cell_corner(c, global_index=self._idx)
        return [cs(i) for i in range(8)]

    def __repr__(self):
        act = 'active' if self.active else 'inactive'
        pos = '(%.3f, %.3f, %.3f)' % self.coordinate
        cnt = '%d, %d, %d, %s, %s, grid=%s' % (self.i, self.j, self.k,
                                               act, pos,
                                               self._grid.get_name())

        return 'Cell(%s)' % cnt

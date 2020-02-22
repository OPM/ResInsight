#  Copyright (C) 2011  Equinor ASA, Norway.
#
#  The file 'ecl_grid.py' is part of ERT - Ensemble based Reservoir Tool.
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
Module to load and query ECLIPSE GRID/EGRID files.

The ecl_grid module contains functionality to load and query an
ECLIPSE grid file; it is currently not possible to manipulate or let
alone create a grid with ecl_grid module. The functionality is
implemented in the EclGrid class. The ecl_grid module is a thin
wrapper around the ecl_grid.c implementation from the libecl library.
"""
import ctypes

import warnings
import numpy
import pandas
import sys
import os.path
import math
import itertools
from cwrap import CFILE, BaseCClass, load, open as copen

from ecl import EclPrototype
from ecl.util.util import monkey_the_camel
from ecl.util.util import IntVector
from ecl import  EclDataType, EclUnitTypeEnum, EclTypeEnum
from ecl.eclfile import EclKW, FortIO
from ecl.grid import Cell


class EclGrid(BaseCClass):
    """
    Class for loading and internalizing ECLIPSE GRID/EGRID files.
    """

    TYPE_NAME = "ecl_grid"
    _fread_alloc                  = EclPrototype("void* ecl_grid_load_case__(char*, bool)", bind = False)
    _grdecl_create                = EclPrototype("ecl_grid_obj ecl_grid_alloc_GRDECL_kw(int, int, int, ecl_kw, ecl_kw, ecl_kw, ecl_kw)", bind = False)
    _alloc_rectangular            = EclPrototype("ecl_grid_obj ecl_grid_alloc_rectangular(int, int, int, double, double, double, int*)", bind = False)
    _exists                       = EclPrototype("bool ecl_grid_exists(char*)", bind = False)

    _get_numbered_lgr             = EclPrototype("ecl_grid_ref ecl_grid_get_lgr_from_lgr_nr(ecl_grid, int)")
    _get_named_lgr                = EclPrototype("ecl_grid_ref ecl_grid_get_lgr(ecl_grid, char*)")
    _get_cell_lgr                 = EclPrototype("ecl_grid_ref ecl_grid_get_cell_lgr1(ecl_grid, int)")
    _num_coarse_groups            = EclPrototype("int  ecl_grid_get_num_coarse_groups(ecl_grid)")
    _in_coarse_group1             = EclPrototype("bool ecl_grid_cell_in_coarse_group1(ecl_grid, int)")
    _free                         = EclPrototype("void ecl_grid_free(ecl_grid)")
    _get_nx                       = EclPrototype("int ecl_grid_get_nx(ecl_grid)")
    _get_ny                       = EclPrototype("int ecl_grid_get_ny(ecl_grid)")
    _get_nz                       = EclPrototype("int ecl_grid_get_nz(ecl_grid)")
    _get_global_size              = EclPrototype("int ecl_grid_get_global_size(ecl_grid)")
    _get_active                   = EclPrototype("int ecl_grid_get_active_size(ecl_grid)")
    _get_active_fracture          = EclPrototype("int ecl_grid_get_nactive_fracture(ecl_grid)")
    _get_name                     = EclPrototype("char* ecl_grid_get_name(ecl_grid)")
    _ijk_valid                    = EclPrototype("bool ecl_grid_ijk_valid(ecl_grid, int, int, int)")
    _get_active_index3            = EclPrototype("int ecl_grid_get_active_index3(ecl_grid, int, int, int)")
    _get_global_index3            = EclPrototype("int ecl_grid_get_global_index3(ecl_grid, int, int, int)")
    _get_active_index1            = EclPrototype("int ecl_grid_get_active_index1(ecl_grid, int)")
    _get_active_fracture_index1   = EclPrototype("int ecl_grid_get_active_fracture_index1(ecl_grid, int)")
    _get_global_index1A           = EclPrototype("int ecl_grid_get_global_index1A(ecl_grid, int)")
    _get_global_index1F           = EclPrototype("int ecl_grid_get_global_index1F(ecl_grid, int)")
    _get_ijk1                     = EclPrototype("void ecl_grid_get_ijk1(ecl_grid, int, int*, int*, int*)")
    _get_ijk1A                    = EclPrototype("void ecl_grid_get_ijk1A(ecl_grid, int, int*, int*, int*)")
    _get_xyz3                     = EclPrototype("void ecl_grid_get_xyz3(ecl_grid, int, int, int, double*, double*, double*)")
    _get_xyz1                     = EclPrototype("void ecl_grid_get_xyz1(ecl_grid, int, double*, double*, double*)")
    _get_cell_corner_xyz1         = EclPrototype("void ecl_grid_get_cell_corner_xyz1(ecl_grid, int, int, double*, double*, double*)")
    _get_corner_xyz               = EclPrototype("void ecl_grid_get_corner_xyz(ecl_grid, int, int, int, double*, double*, double*)")
    _get_xyz1A                    = EclPrototype("void ecl_grid_get_xyz1A(ecl_grid, int, double*, double*, double*)")
    _get_ij_xy                    = EclPrototype("bool ecl_grid_get_ij_from_xy(ecl_grid, double, double, int, int*, int*)")
    _get_ijk_xyz                  = EclPrototype("int  ecl_grid_get_global_index_from_xyz(ecl_grid, double, double, double, int)")
    _cell_contains                = EclPrototype("bool ecl_grid_cell_contains_xyz1(ecl_grid, int, double, double, double)")
    _cell_regular                 = EclPrototype("bool ecl_grid_cell_regular1(ecl_grid, int)")
    _num_lgr                      = EclPrototype("int  ecl_grid_get_num_lgr(ecl_grid)")
    _has_numbered_lgr             = EclPrototype("bool ecl_grid_has_lgr_nr(ecl_grid, int)")
    _has_named_lgr                = EclPrototype("bool ecl_grid_has_lgr(ecl_grid, char*)")
    _grid_value                   = EclPrototype("double ecl_grid_get_property(ecl_grid, ecl_kw, int, int, int)")
    _get_cell_volume              = EclPrototype("double ecl_grid_get_cell_volume1(ecl_grid, int)")
    _get_cell_thickness           = EclPrototype("double ecl_grid_get_cell_thickness1(ecl_grid, int)")
    _get_cell_dx                  = EclPrototype("double ecl_grid_get_cell_dx1(ecl_grid, int)")
    _get_cell_dy                  = EclPrototype("double ecl_grid_get_cell_dy1(ecl_grid, int)")
    _get_depth                    = EclPrototype("double ecl_grid_get_cdepth1(ecl_grid, int)")
    _fwrite_grdecl                = EclPrototype("void   ecl_grid_grdecl_fprintf_kw(ecl_grid, ecl_kw, char*, FILE, double)")
    _load_column                  = EclPrototype("void   ecl_grid_get_column_property(ecl_grid, ecl_kw, int, int, double_vector)")
    _get_top                      = EclPrototype("double ecl_grid_get_top2(ecl_grid, int, int)")
    _get_top1A                    = EclPrototype("double ecl_grid_get_top1A(ecl_grid, int)")
    _get_bottom                   = EclPrototype("double ecl_grid_get_bottom2(ecl_grid, int, int)")
    _locate_depth                 = EclPrototype("int    ecl_grid_locate_depth(ecl_grid, double, int, int)")
    _invalid_cell                 = EclPrototype("bool   ecl_grid_cell_invalid1(ecl_grid, int)")
    _valid_cell                   = EclPrototype("bool   ecl_grid_cell_valid1(ecl_grid, int)")
    _get_distance                 = EclPrototype("void   ecl_grid_get_distance(ecl_grid, int, int, double*, double*, double*)")
    _fprintf_grdecl2              = EclPrototype("void   ecl_grid_fprintf_grdecl2(ecl_grid, FILE, ecl_unit_enum) ")
    _fwrite_GRID2                 = EclPrototype("void   ecl_grid_fwrite_GRID2(ecl_grid, char*, ecl_unit_enum)")
    _fwrite_EGRID2                = EclPrototype("void   ecl_grid_fwrite_EGRID2(ecl_grid, char*, ecl_unit_enum)")
    _equal                        = EclPrototype("bool   ecl_grid_compare(ecl_grid, ecl_grid, bool, bool)")
    _dual_grid                    = EclPrototype("bool   ecl_grid_dual_grid(ecl_grid)")
    _init_actnum                  = EclPrototype("void   ecl_grid_init_actnum_data(ecl_grid, int*)")
    _compressed_kw_copy           = EclPrototype("void   ecl_grid_compressed_kw_copy(ecl_grid, ecl_kw, ecl_kw)")
    _global_kw_copy               = EclPrototype("void   ecl_grid_global_kw_copy(ecl_grid, ecl_kw, ecl_kw)")
    _create_volume_keyword        = EclPrototype("ecl_kw_obj ecl_grid_alloc_volume_kw(ecl_grid, bool)")
    _use_mapaxes                  = EclPrototype("bool ecl_grid_use_mapaxes(ecl_grid)")
    _export_coord                 = EclPrototype("ecl_kw_obj ecl_grid_alloc_coord_kw(ecl_grid)")
    _export_zcorn                 = EclPrototype("ecl_kw_obj ecl_grid_alloc_zcorn_kw(ecl_grid)")
    _export_actnum                = EclPrototype("ecl_kw_obj ecl_grid_alloc_actnum_kw(ecl_grid)")
    _export_mapaxes               = EclPrototype("ecl_kw_obj ecl_grid_alloc_mapaxes_kw(ecl_grid)")
    _get_unit_system              = EclPrototype("ecl_unit_enum ecl_grid_get_unit_system(ecl_grid)")
    _export_index_frame           = EclPrototype("void ecl_grid_export_index(ecl_grid, int*, int*, bool)")
    _export_data_as_int           = EclPrototype("void ecl_grid_export_data_as_int(int, int*, ecl_kw, int*)", bind = False)
    _export_data_as_double        = EclPrototype("void ecl_grid_export_data_as_double(int, int*, ecl_kw, double*)", bind = False)
    _export_volume                = EclPrototype("void ecl_grid_export_volume(ecl_grid, int, int*, double*)")
    _export_position              = EclPrototype("void ecl_grid_export_position(ecl_grid, int, int*, double*)")
    _export_corners               = EclPrototype("void export_corners(ecl_grid, int, int*, double*)")



    @classmethod
    def load_from_grdecl(cls, filename):
        """Will create a new EclGrid instance from grdecl file.

        This function will scan the input file @filename and look for
        the keywords required to build a grid. The following keywords
        are required:

              SPECGRID   ZCORN   COORD

        In addition the function will look for and use the ACTNUM and
        MAPAXES keywords if they are found; if ACTNUM is not found all
        cells are assumed to be active.

        Slightly more exotic grid concepts like dual porosity, NNC
        mapping, LGR and coarsened cells will be completely ignored;
        if you need such concepts you must have an EGRID file and use
        the default EclGrid() constructor - that is also considerably
        faster.
        """

        if os.path.isfile(filename):
            with copen(filename) as f:
                specgrid = EclKW.read_grdecl(f, "SPECGRID", ecl_type=EclDataType.ECL_INT, strict=False)
                zcorn = EclKW.read_grdecl(f, "ZCORN")
                coord = EclKW.read_grdecl(f, "COORD")
                try:
                    actnum = EclKW.read_grdecl(f, "ACTNUM", ecl_type=EclDataType.ECL_INT)
                except ValueError:
                    actnum = None

                try:
                    mapaxes = EclKW.read_grdecl(f, "MAPAXES")
                except ValueError:
                    mapaxes = None

            return EclGrid.create(specgrid, zcorn, coord, actnum, mapaxes)
        else:
            raise IOError("No such file:%s" % filename)

    @classmethod
    def load_from_file(cls, filename):
        """
        Will inspect the @filename argument and create a new EclGrid instance.
        """
        if FortIO.isFortranFile(filename):
            return EclGrid(filename)
        else:
            return EclGrid.loadFromGrdecl(filename)


    @classmethod
    def create(cls, specgrid, zcorn, coord, actnum, mapaxes=None):

        """
        Create a new grid instance from existing keywords.

        This is a class method which can be used to create an EclGrid
        instance based on the EclKW instances @specgrid, @zcorn,
        @coord and @actnum. An ECLIPSE EGRID file contains the
        SPECGRID, ZCORN, COORD and ACTNUM keywords, so a somewhat
        involved way to create a EclGrid instance could be:

          file = ecl.EclFile("ECLIPSE.EGRID")
          specgrid_kw = file.iget_named_kw("SPECGRID", 0)
          zcorn_kw = file.iget_named_kw("ZCORN", 0)
          coord_kw = file.iget_named_kw("COORD", 0)
          actnum_kw = file.iget_named_kw("ACTNUM", 0)

          grid = EclGrid.create(specgrid_kw, zcorn_kw, coord_kw, actnum_kw)

        If you are so inclined ...
        """
        return cls._grdecl_create(specgrid[0], specgrid[1], specgrid[2], zcorn, coord, actnum, mapaxes)

    @classmethod
    def create_rectangular(cls, dims, dV, actnum=None):
        """
        Will create a new rectangular grid. @dims = (nx,ny,nz)  @dVg = (dx,dy,dz)

        With the default value @actnum == None all cells will be active,
        """

        warnings.warn("EclGrid.createRectangular is deprecated. " +
                "Please used the similar method in EclGridGenerator!",
                DeprecationWarning)

        if actnum is None:
            ecl_grid = cls._alloc_rectangular(dims[0], dims[1], dims[2], dV[0], dV[1], dV[2], None)
        else:
            if not isinstance(actnum, IntVector):
                tmp = IntVector(initial_size=len(actnum))
                for (index, value) in enumerate(actnum):
                    tmp[index] = value
                actnum = tmp

            if not len(actnum) == dims[0] * dims[1] * dims[2]:
                raise ValueError("ACTNUM size mismatch: len(ACTNUM):%d  Expected:%d" % (len(actnum), dims[0] * dims[1] * dims[2]))
            ecl_grid = cls._alloc_rectangular(dims[0], dims[1], dims[2], dV[0], dV[1], dV[2], actnum.getDataPtr())

        # If we have not succeeded in creatin the grid we *assume* the
        # error is due to a failed malloc.
        if ecl_grid is None:
            raise MemoryError("Failed to allocated regualar grid")

        return ecl_grid

    def __init__(self, filename, apply_mapaxes=True):
        """
        Will create a grid structure from an EGRID or GRID file.
        """
        c_ptr = self._fread_alloc(filename, apply_mapaxes)
        if c_ptr:
            super(EclGrid, self).__init__(c_ptr)
        else:
            raise IOError("Loading grid from:%s failed" % filename)


    def free(self):
        self._free()

    def _nicename(self):
        """name is often full path to grid, if so, output basename, else name"""
        name = self.getName()
        if os.path.isfile(name):
            name = os.path.basename(name)
        return name

    def __repr__(self):
        """Returns, e.g.:
           EclGrid("NORNE_ATW2013.EGRID", 46x112x22, global_size=113344, active_size=44431) at 0x28c4a70
        """
        name = self._nicename()
        if name:
            name = '"%s", ' % name
        g_size = self.getGlobalSize()
        a_size = self.getNumActive()
        xyz_s  = '%dx%dx%d' % (self.getNX(),self.getNY(),self.getNZ())
        return self._create_repr('%s%s, global_size=%d, active_size=%d' % (name, xyz_s, g_size, a_size))

    def __len__(self):
        """
        len(grid) wil return the total number of cells.
        """
        return self._get_global_size()

    def equal(self, other, include_lgr=True, include_nnc=False, verbose=False):
        """
        Compare the current grid with the other grid.
        """
        if not isinstance(other, EclGrid):
            raise TypeError("The other argument must be an EclGrid instance")
        return self._equal(other, include_lgr, include_nnc, verbose)


    def dual_grid(self):
        """Is this grid dual porosity model?"""
        return self._dual_grid()

    def get_dims(self):
        """A tuple of four elements: (nx, ny, nz, nactive)."""
        return (self.getNX(),
                 self.getNY(),
                 self.getNZ(),
                 self.getNumActive())


    @property
    def nx(self):
        return self._get_nx()

    def get_nx(self):
        """ The number of elements in the x direction"""
        return self._get_nx()

    @property
    def ny(self):
        return self._get_ny()

    def get_ny(self):
        """ The number of elements in the y direction"""
        return self._get_ny()

    @property
    def nz(self):
        return self._get_nz()

    def get_nz(self):
        """ The number of elements in the z direction"""
        return self._get_nz()

    def get_global_size(self):
        """Returns the total number of cells in this grid"""
        return self._get_global_size()

    def get_num_active(self):
        """The number of active cells in the grid."""
        return self._get_active()


    def get_num_active_fracture(self):
        """The number of active cells in the grid."""
        return self._get_active_fracture()


    def get_bounding_box_2d(self, layer=0, lower_left=None, upper_right=None):
        if 0 <= layer <= self.getNZ():
            x = ctypes.c_double()
            y = ctypes.c_double()
            z = ctypes.c_double()

            if lower_left is None:
                i1 = 0
                j1 = 0
            else:
                i1,j1 = lower_left
                if not 0 < i1 < self.getNX():
                    raise ValueError("lower_left i coordinate invalid")

                if not 0 < j1 < self.getNY():
                    raise ValueError("lower_left j coordinate invalid")


            if upper_right is None:
                i2 = self.getNX()
                j2 = self.getNY()
            else:
                i2,j2 = upper_right

                if not 1 < i2 <= self.getNX():
                    raise ValueError("upper_right i coordinate invalid")

                if not 1 < j2 <= self.getNY():
                    raise ValueError("upper_right j coordinate invalid")

            if not i1 < i2:
                raise ValueError("Must have lower_left < upper_right")

            if not j1 < j2:
                raise ValueError("Must have lower_left < upper_right")



            self._get_corner_xyz(i1, j1, layer, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
            p0 = (x.value, y.value)

            self._get_corner_xyz(i2, j1, layer, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
            p1 = (x.value, y.value )

            self._get_corner_xyz( i2, j2, layer, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
            p2 = (x.value, y.value )

            self._get_corner_xyz(i1, j2, layer, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
            p3 = (x.value, y.value )

            return (p0,p1,p2,p3)
        else:
            raise ValueError("Invalid layer value:%d  Valid range: [0,%d]" % (layer, self.getNZ()))


    def get_name(self):
        """
        Name of the current grid, returns a string.

        For the main grid this is the filename given to the
        constructor when loading the grid; for an LGR this is the name
        of the LGR. If the grid instance has been created with the
        create() classmethod this can be None.
        """
        n = self._get_name()
        return str(n) if n else ''

    def cell(self, global_index=None, active_index=None, i=None, j=None, k=None):
        if global_index is not None:
            return Cell(self, global_index)
        if active_index is not None:
            return Cell(self, self.global_index(active_index=active_index))
        if i is not None:
            return Cell(self, self.global_index(ijk=(i,j,k)))

    def __getitem__(self, global_index):
        if isinstance(global_index, tuple):
            i,j,k = global_index
            return self.cell(i=i, j=j, k=k)
        return self.cell(global_index=global_index)

    def __iter__(self):
        for i in range(len(self)):
            yield self[i]

    def cells(self, active=False):
        """Iterator over all the (active) cells"""
        if not active:
            for c in self:
                yield c
        else:
            for i in range(self.get_num_active()):
                yield self.cell(active_index=i)

    def global_index(self, active_index=None, ijk=None):
        """
        Will convert either active_index or (i,j,k) to global index.
        """
        return self.__global_index(active_index=active_index, ijk=ijk)

    def __global_index(self, active_index=None, global_index=None, ijk=None):
        """
        Will convert @active_index or @ijk to global_index.

        This method will convert @active_index or @ijk to a global
        index. Exactly one of the arguments @active_index,
        @global_index or @ijk must be supplied.

        The method is used extensively internally in the EclGrid
        class; most methods which take coordinate input pass through
        this method to normalize the coordinate representation.
        """

        set_count = 0
        if not active_index is None:
            set_count += 1

        if not global_index is None:
            set_count += 1

        if ijk:
            set_count += 1

        if not set_count == 1:
            raise ValueError("Exactly one of the kewyord arguments active_index, global_index or ijk must be set")

        if not active_index is None:
            global_index = self._get_global_index1A( active_index)
        elif ijk:
            nx = self.getNX()
            ny = self.getNY()
            nz = self.getNZ()

            i,j,k = ijk

            if not 0 <= i < nx:
                raise IndexError("Invalid value i:%d  Range: [%d,%d)" % (i, 0, nx))

            if not 0 <= j < ny:
                raise IndexError("Invalid value j:%d  Range: [%d,%d)" % (j, 0, ny))

            if not 0 <= k < nz:
                raise IndexError("Invalid value k:%d  Range: [%d,%d)" % (k, 0, nz))

            global_index = self._get_global_index3(i,j,k)
        else:
            if not 0 <= global_index < self.getGlobalSize():
                raise IndexError("Invalid value global_index:%d  Range: [%d,%d)" % (global_index, 0, self.getGlobalSize()))
        return global_index


    def get_active_index(self, ijk=None, global_index=None):
        """
        Lookup active index based on ijk or global index.

        Will determine the active_index of a cell, based on either
        @ijk = (i,j,k) or @global_index. If the cell specified by the
        input arguments is not active the function will return -1.
        """
        gi = self.__global_index(global_index=global_index, ijk=ijk)
        return self._get_active_index1(gi)


    def get_active_fracture_index(self, ijk=None, global_index=None):
        """
        For dual porosity - get the active fracture index.
        """
        gi = self.__global_index(global_index=global_index, ijk=ijk)
        return self._get_active_fracture_index1(gi)


    def get_global_index1F(self, active_fracture_index):
        """
        Will return the global index corresponding to active fracture index.
        """
        return self._get_global_index1F(active_fracture_index)


    def cell_invalid(self, ijk=None, global_index=None, active_index=None):
        """
        Tries to check if a cell is invalid.

        Cells which are used to represent numerical aquifers are
        typically located in UTM position (0,0); these cells have
        completely whacked up shape and size, and should **NOT** be
        used in calculations involving real world coordinates. To
        protect against this a heuristic is used identify such cells
        and mark them as invalid. There might be other sources than
        numerical aquifers to this problem.
        """
        gi = self.__global_index(global_index=global_index, ijk=ijk, active_index=active_index)
        return self._invalid_cell(gi)


    def valid_cell_geometry(self, ijk=None, global_index=None, active_index=None):
        """Checks if the cell has valid geometry.

        There are at least two reasons why a cell might have invalid
        gemetry:

          1. In the case of GRID files it is not necessary to supply
             the geometry for all the cells; in that case this
             function will return false for cells which do not have
             valid coordinates.

          2. Cells which are used to represent numerical aquifers are
             typically located in UTM position (0,0); these cells have
             completely whacked up shape and size; these cells are
             identified by a heuristic - which might fail

        If the validCellGeometry() returns false for a particular
        cell functions which calculate cell volumes, real world
        coordinates and so on - should not be used.
        """
        gi = self.__global_index(global_index=global_index, ijk=ijk, active_index=active_index)
        return self._valid_cell(gi)



    def active(self, ijk=None, global_index=None):
        """
        Is the cell active?

        See documentation og get_xyz() for explanation of parameters
        @ijk and @global_index.
        """
        gi = self.__global_index(global_index=global_index, ijk=ijk)
        active_index = self._get_active_index1(gi)
        if active_index >= 0:
            return True
        else:
            return False


    def get_global_index(self, ijk=None, active_index=None):
        """
        Lookup global index based on ijk or active index.
        """
        gi = self.__global_index(active_index=active_index, ijk=ijk)
        return gi


    def get_ijk(self, active_index=None, global_index=None):
        """
        Lookup (i,j,k) for a cell, based on either active index or global index.

        The return value is a tuple with three elements (i,j,k).
        """
        i = ctypes.c_int()
        j = ctypes.c_int()
        k = ctypes.c_int()

        gi = self.__global_index(active_index=active_index, global_index=global_index)
        self._get_ijk1(gi, ctypes.byref(i), ctypes.byref(j), ctypes.byref(k))

        return (i.value, j.value, k.value)


    def get_xyz(self, active_index=None, global_index=None, ijk=None):
        """
        Find true position of cell center.

        Will return world position of the center of a cell in the
        grid. The return value is a tuple of three elements:
        (utm_x, utm_y, depth).

        The cells of a grid can be specified in three different ways:

           (i,j,k)      : As a tuple of i,j,k values.

           global_index : A number in the range [0,nx*ny*nz). The
                          global index is related to (i,j,k) as:

                            global_index = i + j*nx + k*nx*ny

           active_index : A number in the range [0,nactive).

        For many of the EclGrid methods a cell can be specified using
        any of these three methods. Observe that one and only method is
        allowed:

        OK:
            pos1 = grid.get_xyz(active_index=100)
            pos2 = grid.get_xyz(ijk=(10,20,7))

        Crash and burn:
            pos3 = grid.get_xyz(ijk=(10,20,7), global_index=10)
            pos4 = grid.get_xyz()

        All the indices in the EclGrid() class are zero offset, this
        is in contrast to ECLIPSE which has an offset 1 interface.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)

        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        self._get_xyz1(gi, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)


    def get_node_pos(self, i, j, k):
        """Will return the (x,y,z) for the node given by (i,j,k).

        Observe that this method does not consider cells, but the
        nodes in the grid. This means that the valid input range for
        i,j and k are are upper end inclusive. To get the four
        bounding points of the lower layer of the grid:

           p0 = grid.getNodePos(0, 0, 0)
           p1 = grid.getNodePos(grid.getNX(), 0, 0)
           p2 = grid.getNodePos(0, grid.getNY(), 0)
           p3 = grid.getNodePos(grid.getNX(), grid.getNY(), 0)

        """
        if not 0 <= i <= self.getNX():
            raise IndexError("Invalid I value:%d - valid range: [0,%d]" % (i, self.getNX()))

        if not 0 <= j <= self.getNY():
            raise IndexError("Invalid J value:%d - valid range: [0,%d]" % (j, self.getNY()))

        if not 0 <= k <= self.getNZ():
            raise IndexError("Invalid K value:%d - valid range: [0,%d]" % (k, self.getNZ()))

        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        self._get_corner_xyz(i,j,k, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)


    def get_cell_corner(self, corner_nr, active_index=None, global_index=None, ijk=None):
        """
        Will look up xyz of corner nr @corner_nr


        lower layer:   upper layer

         2---3           6---7
         |   |           |   |
         0---1           4---5

        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        x = ctypes.c_double()
        y = ctypes.c_double()
        z = ctypes.c_double()
        self._get_cell_corner_xyz1(gi, corner_nr, ctypes.byref(x), ctypes.byref(y), ctypes.byref(z))
        return (x.value, y.value, z.value)

    def get_node_xyz(self, i,j,k):
        """
        This function returns the position of Vertex (i,j,k).

        The coordinates are in the inclusive interval [0,nx] x [0,ny] x [0,nz].
        """
        nx = self.getNX()
        ny = self.getNY()
        nz = self.getNZ()

        corner = 0

        if i == nx:
            i -= 1
            corner += 1

        if j == ny:
            j -= 1
            corner += 2

        if k == nz:
            k -= 1
            corner += 4

        if self._ijk_valid(i, j, k):
            return self.getCellCorner(corner, global_index=i + j*nx + k*nx*ny)
        else:
            raise IndexError("Invalid coordinates: (%d,%d,%d) " % (i,j,k))



    def get_layer_xyz(self, xy_corner, layer):
        nx = self.getNX()

        (j, i) = divmod(xy_corner, nx + 1)
        k = layer
        return self.getNodeXYZ(i,j,k)



    def distance(self, global_index1, global_index2):
        dx = ctypes.c_double()
        dy = ctypes.c_double()
        dz = ctypes.c_double()
        self._get_distance(global_index1, global_index2, ctypes.byref(dx), ctypes.byref(dy), ctypes.byref(dz))
        return (dx.value, dy.value, dz.value)


    def depth(self, active_index=None, global_index=None, ijk=None):
        """
        Depth of the center of a cell.

        Returns the depth of the center of the cell given by
        @active_index, @global_index or @ijk. See method get_xyz() for
        documentation of @active_index, @global_index and @ijk.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        return self._get_depth( gi)

    def top(self, i, j):
        """
        Top of the reservoir; in the column (@i, @j).
        Returns average depth of the four top corners.
        """
        return self._get_top(i, j)

    def top_active(self, i, j):
        """
        Top of the active part of the reservoir; in the column (@i, @j).
        Raises ValueError if (i,j) column is inactive.
        """
        for k in range(self.getNZ()):
            a_idx = self.get_active_index(ijk=(i,j,k))
            if a_idx >= 0:
                return self._get_top1A(a_idx)
        raise ValueError('No active cell in column (%d,%d)' % (i,j))

    def bottom(self, i, j):
        """
        Bottom of the reservoir; in the column (@i, @j).
        """
        return self._get_bottom( i, j)

    def locate_depth(self, depth, i, j):
        """
        Will locate the k value of cell containing specified depth.

        Will scan through the grid column specified by the input
        arguments @i and @j and search for a cell containing the depth
        given by input argument @depth. The return value is the k
        value of cell containing @depth.

        If @depth is above the top of the reservoir the function will
        return -1, and if @depth is below the bottom of the reservoir
        the function will return -nz.
        """
        return self._locate_depth( depth, i, j)


    def find_cell(self, x, y, z, start_ijk=None):
        """
        Lookup cell containg true position (x,y,z).

        Will locate the cell in the grid which contains the true
        position (@x,@y,@z), the return value is as a triplet
        (i,j,k). The underlying C implementation is not veeery
        efficient, and can potentially take quite long time. If you
        provide a good intial guess with the parameter @start_ijk (a
        tuple (i,j,k)) things can speed up quite substantially.

        If the location (@x,@y,@z) can not be found in the grid, the
        method will return None.
        """
        start_index = 0
        if start_ijk:
            start_index = self.__global_index(ijk=start_ijk)

        global_index = self._get_ijk_xyz(x, y, z, start_index)
        if global_index >= 0:
            i = ctypes.c_int()
            j = ctypes.c_int()
            k = ctypes.c_int()
            self._get_ijk1(global_index,
                            ctypes.byref(i), ctypes.byref(j), ctypes.byref(k))
            return (i.value, j.value, k.value)
        return None

    def cell_contains(self, x, y, z, active_index=None, global_index=None, ijk=None):
        """
        Will check if the cell contains point given by world
        coordinates (x,y,z).

        See method get_xyz() for documentation of @active_index,
        @global_index and @ijk.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        return self._cell_contains(gi, x,y,z)


    def find_cell_xy(self, x, y, k):
        """Will find the i,j of cell with utm coordinates x,y.

        The @k input is the layer you are interested in, the allowed
        values for k are [0,nz]. If the coordinates (x,y) are found to
        be outside the grid a ValueError exception is raised.
        """
        if 0 <= k <= self.getNZ():
            i = ctypes.c_int()
            j = ctypes.c_int()
            ok = self._get_ij_xy(x,y,k, ctypes.byref(i), ctypes.byref(j))
            if ok:
                return (i.value, j.value)
            else:
                raise ValueError("Could not find the point:(%g,%g) in layer:%d" % (x,y,k))
        else:
            raise IndexError("Invalid layer value:%d" % k)

    def find_cell_corner_xy(self, x, y, k):
        """Will find the corner nr of corner closest to utm coordinates x,y.

        The @k input is the layer you are interested in, the allowed
        values for k are [0,nz]. If the coordinates (x,y) are found to
        be outside the grid a ValueError exception is raised.
        """
        i,j = self.findCellXY(x,y,k)
        if k == self.getNZ():
            k -= 1
            corner_shift = 4
        else:
            corner_shift = 0

        nx = self.getNX()
        x0,y0,z0 = self.getCellCorner(corner_shift, ijk=(i,j,k))
        d0 = math.sqrt((x0 - x)*(x0 - x) + (y0 - y)*(y0 - y))
        c0 = i + j*(nx + 1)

        x1,y1,z1 = self.getCellCorner(1 + corner_shift, ijk=(i,j,k))
        d1 = math.sqrt((x1 - x)*(x1 - x) + (y1 - y)*(y1 - y))
        c1 = i + 1 + j*(nx + 1)

        x2,y2,z2 = self.getCellCorner(2 + corner_shift, ijk=(i,j,k))
        d2 = math.sqrt((x2 - x)*(x2 - x) + (y2 - y)*(y2 - y))
        c2 = i + (j + 1)*(nx + 1)

        x3,y3,z3 = self.getCellCorner(3 + corner_shift, ijk=(i,j,k))
        d3 = math.sqrt((x3 - x)*(x3 - x) + (y3 - y)*(y3 - y))
        c3 = i + 1 + (j + 1)*(nx + 1)

        l = [(d0, c0), (d1,c1), (d2, c2), (d3,c3)]
        l.sort(key=lambda k: k[0])
        return l[0][1]



    def cell_regular(self, active_index=None, global_index=None, ijk=None):
        """
        The ECLIPSE grid models often contain various degenerate cells,
        which are twisted, have overlapping corners or what not. This
        function gives a moderate sanity check on a cell, essentially
        what the function does is to check if the cell contains it's
        own centerpoint - which is actually not as trivial as it
        sounds.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        return self._cell_regular( gi)


    def cell_volume(self, active_index=None, global_index=None, ijk=None):
        """
        Calculate the volume of a cell.

        Will calculate the total volume of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        return self._get_cell_volume(gi)


    def cell_dz(self, active_index=None, global_index=None, ijk=None):
        """
        The thickness of a cell.

        Will calculate the (average) thickness of the cell. See method
        get_xyz() for documentation of @active_index, @global_index
        and @ijk.
        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        return self._get_cell_thickness( gi)


    def get_cell_dims(self, active_index=None, global_index=None, ijk=None):
        """Will return a tuple (dx,dy,dz) for cell dimension.

        The dx and dy values are best effor estimates of the cell size
        along the i and j directions respectively. The three values
        are guaranteed to satisfy:

              dx * dy * dz = dV

        See method get_xyz() for documentation of @active_index,
        @global_index and @ijk.

        """
        gi = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        dx = self._get_cell_dx(gi)
        dy = self._get_cell_dy(gi)
        dz = self._get_cell_thickness( gi)
        return (dx,dy,dz)



    def get_num_lgr(self):

        """
        How many LGRs are attached to this main grid?

        How many LGRs are attached to this main grid; the grid
        instance doing the query must itself be a main grid.
        """
        return self._num_lgr()



    def has_lgr(self, lgr_name):
        """
        Query if the grid has an LGR with name @lgr_name.
        """
        if self._has_named_lgr(lgr_name):
            return True
        else:
            return False


    def get_lgr(self, lgr_key):
        """Get EclGrid instance with LGR content.

        Return an EclGrid instance based on the LGR @lgr, the input
        argument can either be the name of an LGR or the grid number
        of the LGR. The LGR grid instance is mostly like an ordinary
        grid instance; the only difference is that it can not be used
        for further queries about LGRs.

        If the grid does not contain an LGR with this name/nr
        exception KeyError will be raised.

        """
        lgr = None
        if isinstance(lgr_key, int):
            if self._has_numbered_lgr(lgr_key):
                lgr = self._get_numbered_lgr(lgr_key)
        else:
            if self._has_named_lgr(lgr_key):
                lgr = self._get_named_lgr(lgr_key)

        if lgr is None:
            raise KeyError("No such LGR: %s" % lgr_key)

        lgr.setParent(self)
        return lgr



    def get_cell_lgr(self, active_index=None, global_index=None, ijk=None):
        """
        Get EclGrid instance located in cell.

        Will query the current grid instance if the cell given by
        @active_index, @global_index or @ijk has been refined with an
        LGR. Will return None if the cell in question has not been
        refined, the return value can be used for further queries.

        See get_xyz() for documentation of the input parameters.
        """
        gi  = self.__global_index(ijk=ijk, active_index=active_index, global_index=global_index)
        lgr = self._get_cell_lgr(gi)
        if lgr:
            lgr.setParent(self)
            return lgr
        else:
            raise IndexError("No LGR defined for this cell")


    def grid_value(self, kw, i, j, k):
        """
        Will evalute @kw in location (@i,@j,@k).

        The ECLIPSE properties and solution vectors are stored in
        restart and init files as 1D vectors of length nx*nx*nz or
        nactive. The grid_value() method is a minor convenience
        function to convert the (@i,@j,@k) input values to an
        appropriate 1D index.

        Depending on the length of kw the input arguments are
        converted either to an active index or to a global index. If
        the length of kw does not fit with either the global size of
        the grid or the active size of the grid things will fail hard.
        """
        return self._grid_value(kw, i, j, k)


    def load_column(self, kw, i, j, column):
        """
        Load the values of @kw from the column specified by (@i,@j).

        The method will scan through all k values of the input field
        @kw for fixed values of i and j. The size of @kw must be
        either nactive or nx*ny*nz.

        The input argument @column should be a DoubleVector instance,
        observe that if size of @kw == nactive k values corresponding
        to inactive cells will not be modified in the @column
        instance; in that case it is important that @column is
        initialized with a suitable default value.
        """
        self._load_column( kw, i, j, column)


    def create_kw(self, array, kw_name, pack):
        """
        Creates an EclKW instance based on existing 3D numpy object.

        The method create3D() does the inverse operation; creating a
        3D numpy object from an EclKW instance. If the argument @pack
        is true the resulting keyword will have length 'nactive',
        otherwise the element will have length nx*ny*nz.
        """
        if array.ndim == 3:
            dims = array.shape
            if dims[0] == self.getNX() and dims[1] == self.getNY() and dims[2] == self.getNZ():
                dtype = array.dtype
                if dtype == numpy.int32:
                    type = EclDataType.ECL_INT
                elif dtype == numpy.float32:
                    type = EclDataType.ECL_FLOAT
                elif dtype == numpy.float64:
                    type = EclDataType.ECL_DOUBLE
                else:
                    sys.exit("Do not know how to create ecl_kw from type:%s" % dtype)

                if pack:
                    size = self.getNumActive()
                else:
                    size = self.getGlobalSize()

                if len(kw_name) > 8:
                    # Silently truncate to length 8 - ECLIPSE has it's challenges.
                    kw_name = kw_name[0:8]

                kw = EclKW(kw_name, size, type)
                active_index = 0
                global_index = 0
                for k in range(self.getNZ()):
                    for j in range(self.getNY()):
                        for i in range(self.getNX()):
                            if pack:
                                if self.active(global_index=global_index):
                                    kw[active_index] = array[i,j,k]
                                    active_index += 1
                            else:
                                if dtype == numpy.int32:
                                    kw[global_index] = int(array[i,j,k])
                                else:
                                    kw[global_index] = array[i,j,k]

                            global_index += 1
                return kw
        raise ValueError("Wrong size / dimension on array")


    def coarse_groups(self):
        """
        Will return the number of coarse groups in this grid.
        """
        return self._num_coarse_groups()


    def in_coarse_group(self, global_index=None, ijk=None, active_index=None):
        """
        Will return True or False if the cell is part of coarse group.
        """
        global_index = self.__global_index(active_index=active_index, ijk=ijk, global_index=global_index)
        return self._in_coarse_group1(global_index)


    def create_3d(self, ecl_kw, default = 0):
        """
        Creates a 3D numpy array object with the data from  @ecl_kw.

        Observe that 3D numpy object is a copy of the data in the
        EclKW instance, i.e. modification to the numpy object will not
        be reflected in the ECLIPSE keyword.

        The methods createKW() does the inverse operation; creating an
        EclKW instance from a 3D numpy object.

        Alternative: Creating the numpy array object is not very
        efficient; if you only need a limited number of elements from
        the ecl_kw instance it might be wiser to use the grid_value()
        method:

           value = grid.grid_value(ecl_kw, i, j, k)

        """
        if len(ecl_kw) == self.getNumActive() or len(ecl_kw) == self.getGlobalSize():
            array = numpy.ones([ self.getGlobalSize() ], dtype=ecl_kw.dtype) * default
            kwa = ecl_kw.array
            if len(ecl_kw) == self.getGlobalSize():
                for i in range(kwa.size):
                    array[i] = kwa[i]
            else:
                data_index = 0
                for global_index in range(self.getGlobalSize()):
                    if self.active(global_index=global_index):
                        array[global_index] = kwa[data_index]
                        data_index += 1

            array = array.reshape([self.getNX(), self.getNY(), self.getNZ()], order='F')
            return array
        else:
            err_msg_fmt = 'Keyword "%s" has invalid size %d; must be either nactive=%d or nx*ny*nz=%d'
            err_msg = err_msg_fmt % (ecl_kw, len(ecl_kw), self.getNumActive(),
                                     self.getGlobalSize())
            raise ValueError(err_msg)

    def save_grdecl(self, pyfile, output_unit=EclUnitTypeEnum.ECL_METRIC_UNITS):
        """
        Will write the the grid content as grdecl formatted keywords.

        Will only write the main grid.
        """
        cfile = CFILE(pyfile)
        self._fprintf_grdecl2(cfile, output_unit)

    def save_EGRID(self, filename, output_unit=None):
         if output_unit is None:
             output_unit = self.unit_system
         self._fwrite_EGRID2(filename, output_unit)

    def save_GRID(self, filename, output_unit=EclUnitTypeEnum.ECL_METRIC_UNITS):
        """
        Will save the current grid as a GRID file.
        """
        self._fwrite_GRID2( filename, output_unit)


    def write_grdecl(self, ecl_kw, pyfile, special_header=None, default_value=0):
        """
        Writes an EclKW instance as an ECLIPSE grdecl formatted file.

        The input argument @ecl_kw must be an EclKW instance of size
        nactive or nx*ny*nz. If the size is nactive the inactive cells
        will be filled with @default_value; hence the function will
        always write nx*ny*nz elements.

        The data in the @ecl_kw argument can be of type integer,
        float, double or bool. In the case of bool the default value
        must be specified as 1 (True) or 0 (False).

        The input argument @pyfile should be a valid python filehandle
        opened for writing; i.e.

           pyfile = open("PORO.GRDECL", "w")
           grid.write_grdecl(poro_kw , pyfile, default_value=0.0)
           grid.write_grdecl(permx_kw, pyfile, default_value=0.0)
           pyfile.close()

        """

        if len(ecl_kw) == self.getNumActive() or len(ecl_kw) == self.getGlobalSize():
            cfile = CFILE(pyfile)
            self._fwrite_grdecl(ecl_kw, special_header, cfile, default_value)
        else:
            raise ValueError("Keyword: %s has invalid size(%d), must be either nactive:%d  or nx*ny*nz:%d" % (ecl_kw.getName(), len(ecl_kw), self.getNumActive(), self.getGlobalSize()))


    def exportACTNUM(self):
        actnum = IntVector(initial_size=self.getGlobalSize())
        self._init_actnum(actnum.getDataPtr())
        return actnum


    def compressed_kw_copy(self, kw):
        if len(kw) == self.getNumActive():
            return kw.copy()
        elif len(kw) == self.getGlobalSize():
            kw_copy = EclKW(kw.getName(), self.getNumActive(), kw.data_type)
            self._compressed_kw_copy(kw_copy, kw)
            return kw_copy
        else:
            raise ValueError("The input keyword must have nx*n*nz or nactive elements. Size:%d invalid" % len(kw))

    def global_kw_copy(self, kw, default_value):
        if len(kw) == self.getGlobalSize():
            return kw.copy()
        elif len(kw) == self.getNumActive():
            kw_copy = EclKW(kw.getName(), self.getGlobalSize(), kw.data_type)
            kw_copy.assign(default_value)
            self._global_kw_copy(kw_copy, kw)
            return kw_copy
        else:
            raise ValueError("The input keyword must have nx*n*nz or nactive elements. Size:%d invalid" % len(kw))


    def export_ACTNUM_kw(self):
        actnum = EclKW("ACTNUM", self.getGlobalSize(), EclDataType.ECL_INT)
        self._init_actnum(actnum.getDataPtr())
        return actnum


    def create_volume_keyword(self, active_size=True):
        """Will create a EclKW initialized with cell volumes.

        The purpose of this method is to create a EclKW instance which
        is initialized with all the cell volumes, this can then be
        used to perform volume summation; i.e. to calculate the total
        oil volume:

           soil = 1 - sgas - swat
           cell_volume = grid.createVolumeKeyword()
           tmp = cell_volume * soil
           oip = tmp.sum()

        The oil in place calculation shown above could easily be
        implemented by iterating over the soil kw, however using the
        volume keyword has two advantages:

          1. The calculation of cell volumes is quite time consuming,
             by storing the results in a kw they can be reused.

          2. By using the compact form 'oip = cell_volume * soil' the
             inner loop iteration will go in C - which is faster.

        By default the kw will only have values for the active cells,
        but by setting the optional variable @active_size to False you
        will get volume values for all cells in the grid.
        """

        return self._create_volume_keyword(active_size)

    def export_index(self, active_only = False):
        """
        Exports a pandas dataframe containing index data of grid cells.

        The global_index of the cells is used as index in the pandas frame.
        columns 0, 1, 2 are i, j, k, respectively
        column 3 contains the active_index
        if active_only == True, only active cells are listed, 
        otherwise all cells are listed.
        This index frame should typically be passed to the epxport_data(), 
        export_volume() and export_corners() functions.
        """
        if active_only:
            size = self.get_num_active()
        else:
            size = self.get_global_size()
        indx = numpy.zeros(size, dtype=numpy.int32)
        data = numpy.zeros([size, 4], dtype=numpy.int32)
        self._export_index_frame( indx.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)), data.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)), active_only )
        df = pandas.DataFrame(data=data, index=indx, columns=['i', 'j', 'k', 'active'])
        return df
        
    def export_data(self, index_frame, kw, default = 0):
        """
        Exports keywoard data to a numpy vector. 

        Index_fram must be a pandas dataframe with the same structure 
        as obtained from export_index.
        kw must have size of either global_size or num_active.
        The length of the numpy vector is the number of rows in index_frame.
        If kw is of length num_active, values in the output vector
        corresponding to inactive cells are set to default.
        """
        if not isinstance(index_frame, pandas.DataFrame):
            raise TypeError("index_frame must be pandas.DataFrame")
        if len(kw) == self.get_global_size():
            index = numpy.array( index_frame.index, dtype=numpy.int32 )
        elif len(kw) == self.get_num_active():
            index = numpy.array( index_frame["active"], dtype=numpy.int32 )
        else:
            raise ValueError("The keyword must have a 3D compatible length")

        if kw.type is EclTypeEnum.ECL_INT_TYPE:
            data = numpy.full( len(index), default, dtype=numpy.int32 )
            self._export_data_as_int( len(index), 
                                       index.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)), 
                                       kw, 
                                       data.ctypes.data_as(ctypes.POINTER(ctypes.c_int32))   )
            return data
        elif kw.type is EclTypeEnum.ECL_FLOAT_TYPE or kw.type is EclTypeEnum.ECL_DOUBLE_TYPE:
            data = numpy.full( len(index), default, dtype=numpy.float64 )
            self._export_data_as_double( len(index), 
                                         index.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)), 
                                         kw, 
                                         data.ctypes.data_as(ctypes.POINTER(ctypes.c_double))   )            
            return data
        else:
            raise TypeError("Keyword must be either int, float or double.")

    def export_volume(self, index_frame):
        """
        Exports cell volume data to a numpy vector.

        Index_fram must be a pandas dataframe with the same structure 
        as obtained from export_index.  
        """
        index = numpy.array( index_frame.index, dtype=numpy.int32 )
        data = numpy.zeros( len(index ), dtype=numpy.float64 )
        self._export_volume( len(index), 
                             index.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)),
                             data.ctypes.data_as(ctypes.POINTER(ctypes.c_double))  )
        return data

    def export_position(self, index_frame):
        """Exports cell position coordinates to a numpy vector (matrix), with columns
        0, 1, 2 denoting coordinates x, y, and z, respectively.

        Index_fram must be a pandas dataframe with the same structure 
        as obtained from export_index.  
        """
        index = numpy.array( index_frame.index, dtype=numpy.int32 )
        data = numpy.zeros( [len(index), 3], dtype=numpy.float64 )
        self._export_position( len(index), 
                               index.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)),
                               data.ctypes.data_as(ctypes.POINTER(ctypes.c_double))  )
        return data

    def export_corners(self, index_frame):
        """Exports cell corner position coordinates to a numpy vector (matrix). 
     
        Index_fram must be a pandas dataframe with the same structure 
        as obtained from export_index. 
        Example of a row of the output matrix:
        0   1   2  ....   21   22   23
        x1  y1  z1 ....   x8   y8   z8

        In total there are eight 8 corners. They are described as follows:
        The corners in a cell are numbered 0 - 7, where corners 0-3 constitute
        one layer and the corners 4-7 consitute the other layer. Observe 
        that the numbering does not follow a consistent rotation around the face:


                                        j
        6---7                        /|\    
        |   |                         |
        4---5                         |
                                      |
                                      o---------->  i
        2---3
        |   |
        0---1

        Many grids are left-handed, i.e. the direction of increasing z will
        point down towards the center of the earth. Hence in the figure above
        the layer 4-7 will be deeper down in the reservoir than layer 0-3, and
        also have higher z-value.

        Warning: The main author of this code suspects that the coordinate
        system can be right-handed as well, giving a z axis which will
        increase 'towards the sky'; the safest way is probably to check this
        explicitly if it matters for the case at hand.
        """
        index = numpy.array( index_frame.index, dtype=numpy.int32 )
        data = numpy.zeros( [len(index), 24], dtype=numpy.float64 )
        self._export_corners( len(index),
                              index.ctypes.data_as(ctypes.POINTER(ctypes.c_int32)),
                              data.ctypes.data_as(ctypes.POINTER(ctypes.c_double))  )
        return data


    def export_coord(self):
        return self._export_coord()

    def export_zcorn(self):
        return self._export_zcorn()

    def export_actnum(self):
        return self._export_actnum()

    def export_mapaxes(self):
        if not self._use_mapaxes():
            return None

        return self._export_mapaxes()

    @property
    def unit_system(self):
        return self._get_unit_system()

monkey_the_camel(EclGrid, 'loadFromGrdecl', EclGrid.load_from_grdecl, classmethod)
monkey_the_camel(EclGrid, 'loadFromFile', EclGrid.load_from_file, classmethod)
monkey_the_camel(EclGrid, 'createRectangular', EclGrid.create_rectangular, classmethod)
monkey_the_camel(EclGrid, 'dualGrid', EclGrid.dual_grid)
monkey_the_camel(EclGrid, 'getDims', EclGrid.get_dims)
monkey_the_camel(EclGrid, 'getNX', EclGrid.get_nx)
monkey_the_camel(EclGrid, 'getNY', EclGrid.get_ny)
monkey_the_camel(EclGrid, 'getNZ', EclGrid.get_nz)
monkey_the_camel(EclGrid, 'getGlobalSize', EclGrid.get_global_size)
monkey_the_camel(EclGrid, 'getNumActive', EclGrid.get_num_active)
monkey_the_camel(EclGrid, 'getNumActiveFracture', EclGrid.get_num_active_fracture)
monkey_the_camel(EclGrid, 'getBoundingBox2D', EclGrid.get_bounding_box_2d)
monkey_the_camel(EclGrid, 'getName', EclGrid.get_name)
monkey_the_camel(EclGrid, 'validCellGeometry', EclGrid.valid_cell_geometry)
monkey_the_camel(EclGrid, 'getNodePos', EclGrid.get_node_pos)
monkey_the_camel(EclGrid, 'getCellCorner', EclGrid.get_cell_corner)
monkey_the_camel(EclGrid, 'getNodeXYZ', EclGrid.get_node_xyz)
monkey_the_camel(EclGrid, 'getLayerXYZ', EclGrid.get_layer_xyz)
monkey_the_camel(EclGrid, 'findCellXY', EclGrid.find_cell_xy)
monkey_the_camel(EclGrid, 'findCellCornerXY', EclGrid.find_cell_corner_xy)
monkey_the_camel(EclGrid, 'getCellDims', EclGrid.get_cell_dims)
monkey_the_camel(EclGrid, 'getNumLGR', EclGrid.get_num_lgr)
monkey_the_camel(EclGrid, 'createKW', EclGrid.create_kw)
monkey_the_camel(EclGrid, 'create3D', EclGrid.create_3d)
monkey_the_camel(EclGrid, 'compressedKWCopy', EclGrid.compressed_kw_copy)
monkey_the_camel(EclGrid, 'globalKWCopy', EclGrid.global_kw_copy)
monkey_the_camel(EclGrid, 'exportACTNUMKw', EclGrid.export_ACTNUM_kw)
monkey_the_camel(EclGrid, 'createVolumeKeyword', EclGrid.create_volume_keyword)

#  Copyright (C) 2017  Equinor ASA, Norway.
#
#  The file 'ecl_grid_generator.py' is part of ERT - Ensemble based Reservoir Tool.
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

import itertools, numpy
from math import sqrt

from ecl import EclPrototype
from ecl.util.util import monkey_the_camel
from ecl.util.util import IntVector
from ecl import EclDataType
from ecl.eclfile import EclKW
from ecl.grid import EclGrid

def flatten(l):
    return [elem for sublist in l for elem in sublist]

def divide(l, size):
    return [l[i:i+size:] for i in range(0, len(l), size)]

def duplicate_inner(l):
    return [elem for elem in l for i in range(2)][1:-1:]

def construct_floatKW(name, values):
    kw = EclKW(name, len(values), EclDataType.ECL_FLOAT)
    for i, value in enumerate(values):
        kw[i] = value
    return kw

def pre_mapaxes_translation(translation, mapaxes):
    if mapaxes is None:
        return translation

    x, y, z = translation

    unit_y = numpy.array((mapaxes[0]-mapaxes[2], mapaxes[1]-mapaxes[3]));
    unit_y /= sqrt(numpy.sum(unit_y*unit_y))

    unit_x = numpy.array((mapaxes[4]-mapaxes[2], mapaxes[5]-mapaxes[3]));
    unit_x /= sqrt(numpy.sum(unit_x*unit_x))

    det = 1.0 / (unit_x[0]*unit_y[1] - unit_x[1] * unit_y[0]);

    return (
                ( x*unit_y[1] - y*unit_y[0]) * det,
                (-x*unit_x[1] + y*unit_x[0]) * det,
                z
           )

class EclGridGenerator:

    _alloc_rectangular = EclPrototype("ecl_grid_obj ecl_grid_alloc_rectangular(int, int, int, double, double, double, int*)", bind=False)

    @classmethod
    def create_rectangular(cls, dims, dV, actnum=None):
        """
        Will create a new rectangular grid. @dims = (nx,ny,nz)  @dVg = (dx,dy,dz)

        With the default value @actnum == None all cells will be active,
        """
        if actnum is None:
            ecl_grid = cls._alloc_rectangular(
                                     dims[0], dims[1], dims[2],
                                     dV[0], dV[1], dV[2],
                                     None
                                     )
        else:
            if not isinstance(actnum, IntVector):
                tmp = IntVector(initial_size=len(actnum))
                for (index, value) in enumerate(actnum):
                    tmp[index] = value
                actnum = tmp

            if not len(actnum) == dims[0]*dims[1]*dims[2]:
                raise ValueError(
                        "ACTNUM size mismatch: len(ACTNUM):%d  Expected:%d"
                        % (len(actnum), dims[0]*dims[1]*dims[2])
                        )

            ecl_grid = cls._alloc_rectangular(
                                 dims[0], dims[1], dims[2],
                                 dV[0], dV[1], dV[2],
                                 actnum.getDataPtr()
                                 )

        # If we have not succeeded in creatin the grid we *assume* the
        # error is due to a failed malloc.
        if ecl_grid is None:
            raise MemoryError("Failed to allocated regualar grid")

        return ecl_grid

    @classmethod
    def create_single_cell_grid(cls, corners):
        """
        Provided with the corners of the grid in a similar manner as the eight
        corners are output for a single cell, this method will create a grid
        consisting of a single cell with the specified corners as its corners.
        """

        zcorn = [corners[i][2] for i in range(8)]

        coord = [(corners[i], corners[i+4]) for i in range(4)]
        coord = flatten(flatten(coord))

        def construct_floatKW(name, values):
            kw = EclKW(name, len(values), EclDataType.ECL_FLOAT)
            for i in range(len(values)):
                kw[i] = values[i]
            return kw

        grid = EclGrid.create(
                (1, 1, 1),
                construct_floatKW("ZCORN", zcorn),
                construct_floatKW("COORD", coord),
                None
                )

        if not corners == [grid.getCellCorner(i, 0) for i in range(8)]:
            raise AssertionError("Failed to generate single cell grid. " +
                    "Did not end up the expected corners.")

        return grid

    @classmethod
    def create_zcorn(cls, dims, dV, offset=1, escape_origo_shift=(1,1,0),
            irregular_offset=False, irregular=False, concave=False,
            faults=False):

        cls.__assert_zcorn_parameters(dims, dV, offset, escape_origo_shift,
                    irregular_offset, irregular, concave, faults)

        nx, ny, nz = dims
        dx, dy, dz = dV

        # Compute zcorn
        z = escape_origo_shift[2]
        zcorn = [z]*(4*nx*ny)
        for k in range(nz-1):
            z = z+dz
            local_offset = offset + (dz/2. if irregular_offset and k%2 == 0 else 0)

            layer = []
            for i in range(ny+1):
                shift = ((i if concave else 0) + (k/2 if irregular else 0)) % 2
                path = [z if i%2 == shift else z+local_offset for i in range(nx+1)]
                layer.append(duplicate_inner(path))

            zcorn = zcorn + (2*flatten(duplicate_inner(layer)))

        z = z+dz
        zcorn = zcorn + ([z]*(4*nx*ny))

        if faults:
            # Ensure that drop does not align with grid structure
            drop = (offset+dz)/2. if abs(offset-dz/2.) > 0.2 else offset + 0.4
            zcorn = cls.__create_faults(nx, ny, nz, zcorn, drop)


        if z != escape_origo_shift[2] + nz*dz:
            raise ValueError("%f != %f" % (z, escape_origo_shift[2] + nz*dz))

        cls.assert_zcorn(nx, ny, nz, zcorn)
        return construct_floatKW("ZCORN", zcorn)

    @classmethod
    def create_coord(cls, dims, dV, escape_origo_shift=(1,1,0),
            scale=1, translation=(0,0,0), rotate=False, misalign=False):

        nx, ny, nz = dims
        dx, dy, dz = dV

        # Compute coord
        z = escape_origo_shift[2] + nz*dz
        coord = []
        for j, i in itertools.product(range(ny+1), range(nx+1)):
            x, y = i*dx+escape_origo_shift[0], j*dy+escape_origo_shift[1]
            coord = coord + [x, y, escape_origo_shift[2], x, y, z]

        # Apply transformations
        lower_center = (
                nx*dx/2. + escape_origo_shift[0],
                ny*dy/2. + escape_origo_shift[1]
                )

        if misalign:
            coord = cls.__misalign_coord(coord, dims, dV)

        coord = cls.__scale_coord(coord, scale, lower_center)

        if rotate:
            coord = cls.__rotate_coord(coord, lower_center)

        coord = cls.__translate_lower_coord(coord, translation)

        cls.assert_coord(nx, ny, nz, coord)
        return construct_floatKW("COORD", coord)

    @classmethod
    def __assert_zcorn_parameters(cls, dims, dV, offset, escape_origo_shift,
            irregular_offset, irregular, concave, faults):

        nx, ny, nz = dims
        dx, dy, dz = dV

        # Validate arguments
        if min(dims + dV) <= 0:
            raise ValueError("Expected positive grid and cell dimentions")

        if offset < 0:
            raise ValueError("Expected non-negative offset")

        if irregular and offset + (dz/2. if irregular_offset else 0) > dz:
            raise AssertionError("Arguments can result in self-" +
                    "intersecting cells. Increase dz, deactivate eiter " +
                    "irregular or irregular_offset, or decrease offset to avoid " +
                    "any problems")

    @classmethod
    def create_grid(cls, dims, dV, offset=1,
            escape_origo_shift=(1,1,0),
            irregular_offset=False, irregular=False, concave=False,
            faults=False, scale=1, translation=(0,0,0), rotate=False,
            misalign=False):
        """
        Will create a new grid where each cell is a parallelogram (skewed by z-value).
        The number of cells are given by @dims = (nx, ny, nz) and the dimention
        of each cell by @dV = (dx, dy, dz).

        All cells are guaranteed to not be self-intersecting. Hence, no twisted
        cells and somewhat meaningfull cells.

        @offset gives how much the layers should fluctuate or "wave" as you
        move along the X-axis.

        @irregular_offset decides whether the offset should be constant or
        increase by dz/2 every now and then.

        @irregular if true some of the layers will be inclining and others
        declining at the start.

        @concave decides whether the cells are to be convex or not. In
        particular, if set to False, all cells of the grid will be concave.

        @escape_origo_shift is used to prevent any cell of having corners in (0,0,z)
        as there is a heuristic in ecl_grid.c that marks such cells as tainted.

        @faults decides if there are to be faults in the grid.

        @scale A positive number that scales the "lower" endpoint of all
        coord's. In particular, @scale != 1 creates trapeziod cells in both the XZ
        and YZ-plane.

        @translation the lower part of the grid is translated ("slided") by the specified
        additive factor.

        @rotate the lower part of the grid is rotated 90 degrees around its
        center.

        @misalign will toggle COORD's slightly in various directions to break
        alignment

        Note that cells in the lowermost layer can have multiple corners
        at the same point.

        For testing it should give good coverage of the various scenarios this
        method can produce, by leting @dims be (10,10,10), @dV=(2,2,2), @offset=1,
        and try all 4 different configurations of @concave and
        @irregular_offset.
        """

        zcorn = cls.create_zcorn(dims, dV, offset, escape_origo_shift,
                                irregular_offset, irregular, concave, faults)

        coord = cls.create_coord(dims, dV, escape_origo_shift, scale,
                                translation, rotate, misalign)

        return EclGrid.create(dims, zcorn, coord, None)

    @classmethod
    def __create_faults(cls, nx, ny, nz, zcorn, drop):
        """
        Will create several faults consisting of all cells such that either its
        i or j index is 1 modulo 3.
        """

        plane_size = 4*nx*ny
        for x, y, z in itertools.product(range(nx), range(ny), range(nz)):
            if x%3 != 1 and y%3 != 1:
                continue

            corner = [0]*8
            corner[0] = 2*z*plane_size + 4*y*nx + 2*x
            corner[1] = corner[0] + 1
            corner[2] = corner[0] + 2*nx
            corner[3] = corner[2] + 1

            for i in range(4, 8):
                corner[i] = corner[i-4] + plane_size

            for c in corner:
                zcorn[c] = zcorn[c] + drop

        return zcorn

    @classmethod
    def assert_zcorn(cls, nx, ny, nz, zcorn, twisted_check=True):
        """

        Raises an AssertionError if the zcorn is not as expected. In
        patricular, it is verified that:

            - zcorn has the approperiate length (8*nx*ny*nz) and
            - that no cell is twisted.

        """

        if len(zcorn) != 8*nx*ny*nz:
            raise AssertionError(
                    "Expected len(zcorn) to be %d, was %d" %
                    (8*nx*ny*nz, len(zcorn))
                    )

        plane_size = 4*nx*ny
        for p in range(8*nx*ny*nz - plane_size):
            if zcorn[p] > zcorn[p + plane_size] and twisted_check:
                raise AssertionError(
                    "Twisted cell was created. " +
                    "Decrease offset or increase dz to avoid this!"
                    )

    @classmethod
    def __scale_coord(cls, coord, scale, lower_center):
        coord = numpy.array([
            list(map(float, coord[i:i+6:]))
            for i in range(0, len(coord), 6)
            ])
        origo = numpy.array(3*[0.] + list(lower_center) + [0])
        scale = numpy.array(3*[1.] + 2*[scale] + [1])
        coord = scale * (coord-origo) + origo
        return coord.flatten().tolist()

    @classmethod
    def __misalign_coord(cls, coord, dims, dV):
        nx, ny, nz = dims

        coord = numpy.array([
            list(map(float, coord[i:i+6:]))
            for i in range(0, len(coord), 6)
            ])

        tf = lambda i, j: 1./2 if abs(i)+abs(j) <= 1 else 0.25
        adjustment = numpy.array([
            (0, 0, 0, i*tf(i,j)*dV[0], j*tf(i,j)*dV[1], 0) for i, j in itertools.product([-1, 0, 1], repeat=2)
            ])

        for i, c in enumerate(coord):
            # Leave the outermost coords alone
            if i < nx+1 or i >= len(coord)-(nx+1):
                continue
            if i%(nx+1) in [0, nx]:
                continue

            c += adjustment[i%len(adjustment)]

        return coord.flatten().tolist()

    @classmethod
    def __rotate_coord(cls, coord, lower_center):
        coord = numpy.array([
            list(map(float, coord[i:i+6:]))
            for i in range(0, len(coord), 6)
            ])

        origo = numpy.array(3*[0.] + list(lower_center) + [0])
        coord -= origo

        for c in coord:
            c[3], c[4] = -c[4], c[3]

        coord += origo
        return coord.flatten().tolist()

    @classmethod
    def __translate_lower_coord(cls, coord, translation):
        coord = numpy.array([
            list(map(float, coord[i:i+6:]))
            for i in range(0, len(coord), 6)
            ])
        translation = numpy.array(3*[0.] + list(translation))

        coord = coord + translation
        return coord.flatten().tolist()

    @classmethod
    def assert_coord(cls, nx, ny, nz, coord, negative_values=False):
        """

        Raises an AssertionError if the coord is not as expected. In
        particular, it is verified that:

            - coord has the approperiate length (6*(nx+1)*(ny+1)) and
            - that all values are positive unless negative_values are
              explicitly allowed.

        """

        if len(coord) != 6*(nx+1)*(ny+1):
            raise AssertionError(
                    "Expected len(coord) to be %d, was %d" %
                    (6*(nx+1)*(ny+1), len(coord))
                    )

        if not negative_values and min(coord) < 0:
            raise AssertionError("Negative COORD values was generated. " +
                    "This is likely due to a tranformation. " +
                    "Increasing the escape_origio_shift will most likely " +
                    "fix the problem")

    @classmethod
    def assert_actnum(cls, nx, ny, nz, actnum):
        """

        Raises an AssertionError if the actnum is not as expected. In
        particular, it is verified that:

            - actnum has the approperiate length nx*ny*nz and
            - that all values are either 0 or 1.

        """

        if actnum is None:
            return

        if len(actnum) != nx*ny*nz:
            raise AssertionError(
                    "Expected the length of ACTNUM to be %d, was %s."
                    %(nx*ny*nz, len(actnum))
                    )

        if set(actnum)-set([0,1]):
            raise AssertionError(
                "Expected ACTNUM to consist of 0's and 1's, was %s."
                % ", ".join(map(str, set(actnum)))
                )

    @classmethod
    def extract_coord(cls, dims, coord, ijk_bounds):
        nx, ny, nz = dims
        (lx, ux), (ly, uy), (lz, uz) = ijk_bounds
        new_nx, new_ny, new_nz = ux-lx+1, uy-ly+1, uz-lz+1

        cls.assert_coord(nx, ny, nz, coord, negative_values=True)

        # Format COORD
        coord = divide(divide(coord, 6), nx+1)

        # Extract new COORD
        new_coord = [coord_slice[lx:ux+2:]
                        for coord_slice in coord[ly:uy+2]]

        # Flatten and verify
        new_coord = flatten(flatten(new_coord))
        cls.assert_coord(new_nx, new_ny, new_nz, new_coord,
                negative_values=True)

        return construct_floatKW("COORD", new_coord)

    @classmethod
    def extract_zcorn(cls, dims, zcorn, ijk_bounds):
        nx, ny, nz = dims
        (lx, ux), (ly, uy), (lz, uz) = ijk_bounds
        new_nx, new_ny, new_nz = ux-lx+1, uy-ly+1, uz-lz+1

        cls.assert_zcorn(nx, ny, nz, zcorn, twisted_check=False)

        # Format ZCORN
        zcorn = divide(divide(zcorn, 2*nx), 2*ny)

        # Extract new ZCORN
        new_zcorn = [
                        y_slice[2*lx:2*ux+2:]
                        for z_slice in zcorn[2*lz:2*uz+2:]
                        for y_slice in z_slice[2*ly:2*uy+2:]
                    ]

        # Flatten and verify
        new_zcorn = flatten(new_zcorn)
        cls.assert_zcorn(new_nx, new_ny, new_nz, new_zcorn)

        return construct_floatKW("ZCORN", new_zcorn)

    @classmethod
    def extract_actnum(cls, dims, actnum, ijk_bounds):
        if actnum is None:
            return None

        nx, ny, nz = dims
        (lx, ux), (ly, uy), (lz, uz) = ijk_bounds
        new_nx, new_ny, new_nz = ux-lx+1, uy-ly+1, uz-lz+1

        cls.assert_actnum(nx, ny, nz, actnum)

        actnum = divide(divide(actnum, nx), ny)

        new_actnum = [
                        y_slice[lx:ux+1:]
                        for z_slice in actnum[lz:uz+1:]
                        for y_slice in z_slice[ly:uy+1:]
                    ]

        new_actnum = flatten(new_actnum)
        cls.assert_actnum(new_nx, new_ny, new_nz, new_actnum)

        actnumkw = EclKW("ACTNUM", len(new_actnum), EclDataType.ECL_INT)
        for i, value in enumerate(new_actnum):
            actnumkw[i] = value

        return actnumkw

    @classmethod
    def __translate_coord(cls, coord, translation):
        coord = numpy.array([
            list(map(float, coord[i:i+6:]))
            for i in range(0, len(coord), 6)
            ])
        translation = numpy.array(list(translation) + list(translation))

        coord = coord + translation
        return construct_floatKW("COORD", coord.flatten().tolist())


    @classmethod
    def extract_subgrid(cls, grid, ijk_bounds,
            decomposition_change=False, translation=None):

        """
        Extracts a subgrid from the given grid according to the specified
        bounds.

        @ijk_bounds: The bounds describing the subgrid. Should be a tuple of
        length 3, where each element gives the bound for the i, j, k
        coordinates of the subgrid to be described, respectively. Each bound
        should either be an interval of the form (a, b) where 0 <= a <= b < nx
        or a single integer a which is equivialent to the bound (a, a).

        NOTE: The given bounds are including endpoints.

        @decomposition_change: Depending on the given ijk_bounds, libecl might
        decompose the cells of the subgrid differently when extracted from
        grid. This is somewhat unexpected behaviour and if this event occur we
        give an exception together with an description for how to avoid this,
        unless decompostion_change is set to True.

        @translation: Gives the possibility of translating the subgrid. Should
        be given as a tuple (dx, dy, dz), where each coordinate of the grid
        will be moved by di in direction i.

        """

        gdims = grid.getDims()[:-1:]
        nx, ny, nz = gdims
        ijk_bounds = cls.assert_ijk_bounds(gdims, ijk_bounds)

        coord = grid.export_coord()
        cls.assert_coord(nx, ny, nz, coord, negative_values=True)

        zcorn = grid.export_zcorn()
        cls.assert_zcorn(nx, ny, nz, zcorn)

        actnum = grid.export_actnum()
        cls.assert_actnum(nx, ny, nz, actnum)

        mapaxes = grid.export_mapaxes()

        sub_data = cls.extract_subgrid_data(
                                    gdims,
                                    coord, zcorn,
                                    ijk_bounds=ijk_bounds,
                                    actnum=actnum,
                                    mapaxes=mapaxes,
                                    decomposition_change=decomposition_change,
                                    translation=translation
                                    )

        sdim = tuple([b-a+1 for a,b in ijk_bounds])
        sub_coord, sub_zcorn, sub_actnum = sub_data

        return EclGrid.create(sdim, sub_zcorn, sub_coord, sub_actnum, mapaxes=mapaxes)

    @classmethod
    def extract_subgrid_data(cls, dims, coord, zcorn, ijk_bounds, actnum=None,
            mapaxes=None, decomposition_change=False, translation=None):
        """

        Extracts subgrid data from COORD, ZCORN and potentially ACTNUM. It
        returns similar formatted data for the subgrid described by the bounds.

        @dims: The dimentions (nx, ny, nz) of the grid

        @coord: The COORD data of the grid.

        @zcorn: The ZCORN data of the grid.

        @ijk_bounds: The bounds describing the subgrid. Should be a tuple of
        length 3, where each element gives the bound for the i, j, k
        coordinates of the subgrid to be described, respectively. Each bound
        should either be an interval of the form (a, b) where 0 <= a <= b < nx
        or a single integer a which is equivialent to the bound (a, a).

        NOTE: The given bounds are including endpoints.

        @actnum: The ACTNUM data of the grid.

        @mapaxes The MAPAXES data of the grid.

        @decomposition_change: Depending on the given ijk_bounds, libecl might
        decompose the cells of the subgrid differently when extracted from
        grid. This is somewhat unexpected behaviour and if this event occur we
        give an exception together with an description for how to avoid this,
        unless decompostion_change is set to True.

        @translation: Gives the possibility of translating the subgrid. Should
        be given as a tuple (dx, dy, dz), where each coordinate of the grid
        will be moved by di in direction i.

        """
        coord, zcorn = list(coord), list(zcorn)
        actnum = None if actnum is None else list(actnum)

        ijk_bounds = cls.assert_ijk_bounds(dims, ijk_bounds)
        cls.assert_decomposition_change(ijk_bounds, decomposition_change)

        nx, ny, nz = dims
        (lx, ux), (ly, uy), (lz, uz) = ijk_bounds
        new_nx, new_ny, new_nz = ux-lx+1, uy-ly+1, uz-lz+1

        new_coord = cls.extract_coord(dims, coord, ijk_bounds)
        new_zcorn = cls.extract_zcorn(dims, zcorn, ijk_bounds)
        new_actnum = cls.extract_actnum(dims, actnum, ijk_bounds)

        if translation is not None:
            mtranslation = pre_mapaxes_translation(translation, mapaxes)
            new_coord = cls.__translate_coord(new_coord, mtranslation)

            for i in range(len(new_zcorn)):
                new_zcorn[i] += translation[2]

        return new_coord, new_zcorn, new_actnum

    @classmethod
    def assert_ijk_bounds(cls, dims, ijk_bounds):
        ijk_bounds = list(ijk_bounds)

        for i in range(len(ijk_bounds)):
            if isinstance(ijk_bounds[i], int):
                ijk_bounds[i] = [ijk_bounds[i]]
            if len(ijk_bounds[i]) == 1:
                ijk_bounds[i] += ijk_bounds[i]

        if len(ijk_bounds) != 3:
            raise ValueError(
                    "Expected ijk_bounds to contain three intervals, " +
                    "contained only %d" % len(ijk_bounds))

        for n, bound in zip(dims, ijk_bounds):
            if len(bound) != 2:
                raise ValueError(
                    "Expected bound to consist of two elements, was %s",
                    str(bound))

            if not (isinstance(bound[0], int) and isinstance(bound[1], int)):
                raise TypeError(
                    "Expected bound to consist of two integers, ",
                    "was %s (%s)"
                    %(str(bound), str((map(type,bound))))
                    )

            if not (0 <= bound[0] <= bound[1] < n):
                raise ValueError(
                    "Expected bounds to have the following format: " +
                    "0 <= lower bound <= upper_bound < ni, "+
                    "was %d <=? %d <=? %d <? %d."
                    % (0, bound[0], bound[1], n))

        return ijk_bounds

    @classmethod
    def assert_decomposition_change(cls, ijk_bounds, decomposition_change):
        if sum(list(zip(*ijk_bounds))[0]) % 2 == 1 and not decomposition_change:
            raise ValueError(
                    "The subgrid defined by %s " % str(ijk_bounds) +
                    "will cause an unintended decomposition change. " +
                    "Either change one of the lower bounds by 1 " +
                    "or activate decomposition_change."
                    )

monkey_the_camel(EclGridGenerator, 'createRectangular', EclGridGenerator.create_rectangular, classmethod)

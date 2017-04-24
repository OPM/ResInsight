#  Copyright (C) 2017  Statoil ASA, Norway.
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

import itertools
import numpy

from ert.util import IntVector
from ert.ecl import EclGrid, EclKW, EclDataType, EclPrototype

class EclGridGenerator:

    _alloc_rectangular = EclPrototype("ecl_grid_obj ecl_grid_alloc_rectangular( int , int , int , double , double , double , int*)" , bind = False)

    @classmethod
    def createRectangular(cls, dims , dV , actnum = None):
        """
        Will create a new rectangular grid. @dims = (nx,ny,nz)  @dVg = (dx,dy,dz)

        With the default value @actnum == None all cells will be active,
        """
        if actnum is None:
            ecl_grid = cls._alloc_rectangular( dims[0] , dims[1] , dims[2] , dV[0] , dV[1] , dV[2] , None )
        else:
            if not isinstance(actnum , IntVector):
                tmp = IntVector(initial_size = len(actnum))
                for (index , value) in enumerate(actnum):
                    tmp[index] = value
                actnum = tmp

            if not len(actnum) == dims[0] * dims[1] * dims[2]:
                raise ValueError("ACTNUM size mismatch: len(ACTNUM):%d  Expected:%d" % (len(actnum) , dims[0] * dims[1] * dims[2]))
            ecl_grid = cls._alloc_rectangular( dims[0] , dims[1] , dims[2] , dV[0] , dV[1] , dV[2] , actnum.getDataPtr() )

        # If we have not succeeded in creatin the grid we *assume* the
        # error is due to a failed malloc.
        if ecl_grid is None:
            raise MemoryError("Failed to allocated regualar grid")
            
        return ecl_grid

    @classmethod
    def createSingleCellGrid(cls, corners):
        """
        Provided with the corners of the grid in a similar manner as the eight
        corners are output for a single cell, this method will create a grid
        consisting of a single cell with the specified corners as its corners.
        """

        zcorn = [corners[i][2] for i in range(8)]

        flatten = lambda l : [elem for sublist in l for elem in sublist]
        coord = [(corners[i], corners[i+4]) for i in range(4)]
        coord = flatten(flatten(coord))

        def constructFloatKW(name, values):
            kw = EclKW(name, len(values), EclDataType.ECL_FLOAT)
            for i in range(len(values)):
                kw[i] = values[i]
            return kw

        grid = EclGrid.create((1,1,1), constructFloatKW("ZCORN", zcorn), constructFloatKW("COORD", coord), None)

        if not corners == [grid.getCellCorner(i, 0) for i in range(8)]:
            raise AssertionError("Failed to generate single cell grid. " +
                    "Did not end up the expected corners.")

        return grid

    @classmethod
    def createGrid(cls, dims, dV, offset=1,
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

        verbose = lambda l : [elem for elem in l for i in range(2)][1:-1:]
        flatten = lambda l : [elem for sublist in l for elem in sublist]

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
                layer.append(verbose(path))

            zcorn = zcorn + (2*flatten(verbose(layer)))

        z = z+dz
        zcorn = zcorn + ([z]*(4*nx*ny))

        if faults:
            # Ensure that drop does not align with grid structure
            drop = (offset+dz)/2. if abs(offset-dz/2.) > 0.2 else offset + 0.4
            zcorn = cls.__createFaults(nx, ny, nz, zcorn, drop)

        cls.assertZcorn(nx, ny, nz, zcorn)

        # Compute coord
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
            coord = cls.__misalignCoord(coord, dims, dV)

        coord = cls.__scaleCoord(coord, scale, lower_center)

        if rotate:
            coord = cls.__rotateCoord(coord, lower_center)

        coord = cls.__translateCoord(coord, translation)

        cls.assertCoord(nx, ny, nz, coord)

        # Construct grid
        def constructFloatKW(name, values):
            kw = EclKW(name, len(values), EclDataType.ECL_FLOAT)
            for i in range(len(values)):
                kw[i] = values[i]
            return kw

        return EclGrid.create(dims, constructFloatKW("ZCORN", zcorn), constructFloatKW("COORD", coord), None)

    @classmethod
    def __createFaults(cls, nx, ny, nz, zcorn, drop):
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
    def assertZcorn(cls, nx, ny, nz, zcorn):
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
            if zcorn[p] > zcorn[p + plane_size]:
                raise AssertionError(
                    "Twisted cell was created. " +
                    "Decrease offset or increase dz to avoid this!"
                    )

    @classmethod
    def __scaleCoord(cls, coord, scale, lower_center):
        coord = numpy.array([
            map(float, coord[i:i+6:])
            for i in range(0, len(coord), 6)
            ])
        origo = numpy.array(3*[0.] + list(lower_center) + [0])
        scale = numpy.array(3*[1.] + 2*[scale] + [1])

        coord = scale * (coord-origo) + origo
        return coord.flatten().tolist()

    @classmethod
    def __misalignCoord(cls, coord, dims, dV):
        nx, ny, nz = dims

        coord = numpy.array([
            map(float, coord[i:i+6:])
            for i in range(0, len(coord), 6)
            ])

        adjustment = numpy.array([
            (0, 0, 0, i*dV[0]/2., j*dV[1]/2., 0) for i, j in itertools.product([-1, 0, 1], repeat=2)
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
    def __rotateCoord(cls, coord, lower_center):
        coord = numpy.array([
            map(float, coord[i:i+6:])
            for i in range(0, len(coord), 6)
            ])

        origo = numpy.array(3*[0.] + list(lower_center) + [0])
        coord -= origo

        for c in coord:
            c[3], c[4] = -c[4], c[3]

        coord += origo
        return coord.flatten().tolist()

    @classmethod
    def __translateCoord(cls, coord, translation):
        coord = numpy.array([
            map(float, coord[i:i+6:])
            for i in range(0, len(coord), 6)
            ])
        translation = numpy.array(3*[0.] + list(translation))

        coord = coord + translation
        return coord.flatten().tolist()

    @classmethod
    def assertCoord(cls, nx, ny, nz, coord):
        """

        Raises an AssertionError if the coord is not as expected. In
        particular, it is verfied that:

            - coord has the approperiate length (6*(nx+1)*(ny+1)) and
            - that all values are positive.

        """

        if len(coord) != 6*(nx+1)*(ny+1):
            raise AssertionError(
                    "Expected len(coord) to be %d, was %d" %
                    (6*(nx+1)*(ny+1), len(coord))
                    )

        if min(coord) < 0:
            raise AssertionError("Negative COORD values was generated. " +
                    "This is likely due to a tranformation. " +
                    "Increasing the escape_origio_shift will most likely " +
                    "fix the problem")



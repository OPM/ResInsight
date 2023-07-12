# pylint: disable=too-few-public-methods

"""
Module containing the Grid class, containing information
about Case grids.
"""

import Case_pb2
import Grid_pb2
import Grid_pb2_grpc
import Definitions_pb2

from typing import Tuple, Optional, List
from grpc import Channel

from .case import Case


class Grid:
    """Grid Information. Created by methods in Case
    :meth:`rips.case.grid()`
    :meth:`rips.case.grids()`
    """

    def __init__(self, index: int, case: Case, channel: Channel) -> None:
        self.__channel = channel
        self.__stub = Grid_pb2_grpc.GridStub(self.__channel)

        self.case: Case = case
        self.index: int = index
        self.cached_dimensions = None

    def dimensions(self) -> Optional[Definitions_pb2.Vec3i]:
        """The dimensions in i, j, k direction

        Returns:
            Vec3i: class with integer attributes i, j, k giving extent in all three dimensions.
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)

        if self.cached_dimensions is None:
            self.cached_dimensions = self.__stub.GetDimensions(
                Grid_pb2.GridRequest(case_request=case_request, grid_index=self.index)
            ).dimensions

        return self.cached_dimensions

    def cell_centers_async(self):
        """The cells center for all cells in given grid async.

        Returns:
            Iterator to a list of Vec3d: class with double attributes x, y, x giving cell centers
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)
        chunks = self.__stub.GetCellCenters(
            Grid_pb2.GridRequest(case_request=case_request, grid_index=self.index)
        )
        for chunk in chunks:
            yield chunk

    def cell_centers(self) -> List[Definitions_pb2.Vec3d]:
        """The cell center for all cells in given grid

        Returns:
            List of Vec3d: class with double attributes x, y, x giving cell centers
        """
        centers = []
        chunks = self.cell_centers_async()
        for chunk in chunks:
            for center in chunk.centers:
                centers.append(center)
        return centers

    def cell_corners_async(self):
        """The cell corners for all cells in given grid, async.

        Returns:
            iterator to a list of CellCorners: a class with Vec3d for each corner (c0, c1.., c7)
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)
        chunks = self.__stub.GetCellCorners(
            Grid_pb2.GridRequest(case_request=case_request, grid_index=self.index)
        )

        for chunk in chunks:
            yield chunk

    def cell_corners(self):
        """The cell corners for all cells in given grid

        Returns:
            list of CellCorners: a class with Vec3d for each corner (c0, c1.., c7)
        """
        corners = []
        chunks = self.cell_corners_async()
        for chunk in chunks:
            for center in chunk.cells:
                corners.append(center)
        return corners

    def property_data_index_from_ijk(self, i: int, j: int, k: int) -> int:
        """Compute property index from 1-based IJK cell address. Cell Property Result data is organized by I, J and K.

        property_data_index = dims.i * dims.j * (k - 1) + dims.i * (j - 1) + (i - 1)

        Returns:
            int: Cell property result index from IJK
        """

        dims = self.dimensions()
        if dims:
            return int(dims.i * dims.j * (k - 1) + dims.i * (j - 1) + (i - 1))
        else:
            return -1

    def cell_count(self) -> int:
        """Cell count in grid

        Returns:
            int: Cell count in grid
        """

        dims = self.dimensions()
        if dims:
            return int(dims.i * dims.j * dims.k)
        else:
            return 0

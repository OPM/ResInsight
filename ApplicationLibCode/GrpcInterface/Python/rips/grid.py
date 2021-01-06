# pylint: disable=too-few-public-methods

"""
Module containing the Grid class, containing information
about Case grids.
"""

import Case_pb2
import Grid_pb2
import Grid_pb2_grpc


class Grid:
    """Grid Information. Created by methods in Case
    :meth:`rips.case.grid()`
    :meth:`rips.case.grids()`
    """

    def __init__(self, index, case, channel):
        self.__channel = channel
        self.__stub = Grid_pb2_grpc.GridStub(self.__channel)

        self.case = case
        self.index = index

    def dimensions(self):
        """The dimensions in i, j, k direction

        Returns:
            Vec3i: class with integer attributes i, j, k giving extent in all three dimensions.
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)
        return self.__stub.GetDimensions(
            Grid_pb2.GridRequest(case_request=case_request,
                                 grid_index=self.index)).dimensions

    def cell_centers_async(self):
        """The cells center for all cells in given grid async.

        Returns:
            Iterator to a list of Vec3d: class with double attributes x, y, x giving cell centers
        """
        case_request = Case_pb2.CaseRequest(id=self.case.id)
        chunks = self.__stub.GetCellCenters(
            Grid_pb2.GridRequest(case_request=case_request,
                                 grid_index=self.index))
        for chunk in chunks:
            yield chunk

    def cell_centers(self):
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
            Grid_pb2.GridRequest(case_request=case_request,
                                 grid_index=self.index))

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

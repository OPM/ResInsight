# pylint: disable=too-few-public-methods

"""
Module containing the Grid class, containing information
about Case grids.
"""

import rips.generated.Case_pb2 as Case_pb2
import rips.generated.Grid_pb2 as Grid_pb2
import rips.generated.Grid_pb2_grpc as Grid_pb2_grpc


class Grid:
    """Grid Information. Not meant to be constructed separately

    Create Grid objects using methods on Case: Grid() and Grids()
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
        case_request = Case_pb2.CaseRequest(id=self.case.case_id)
        return self.__stub.GetDimensions(
            Grid_pb2.GridRequest(case_request=case_request,
                                 grid_index=self.index)).dimensions


    def cell_centers_async(self):
        """The cells center for all cells in given grid async.

        Returns:
            Iterator to a list of Vec3d: class with double attributes x, y, x giving cell centers
        """
        case_request = Case_pb2.CaseRequest(id=self.case.case_id)
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

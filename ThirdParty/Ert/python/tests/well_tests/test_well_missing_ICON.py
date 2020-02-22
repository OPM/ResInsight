import datetime
from tests import EclTest
from ecl.grid import EclGridGenerator
from ecl.well import WellInfo, WellConnection, WellTypeEnum, WellConnectionDirectionEnum, WellSegment




class EclWellICONTest(EclTest):

    def setUp(self):
        self.grid = EclGridGenerator.create_rectangular((46,112,22), (1,1,1))
        self.rst_file_ICON0 = self.createTestPath("local/ECLIPSE/well/missing-ICON/ICON0.X0027")
        self.rst_file_ICON1 = self.createTestPath("local/ECLIPSE/well/missing-ICON/ICON1.X0027")


    def check_connections(self, well_info, expected):
        well = well_info["B-2H"]
        well_state = well[0]
        self.assertEqual( well_state.hasGlobalConnections(), expected)


    def test_missing_icon(self):
        well_info_ICON0 = WellInfo(self.grid, self.rst_file_ICON0)
        well_info_ICON1 = WellInfo(self.grid, self.rst_file_ICON1)

        self.check_connections(well_info_ICON0, False)
        self.check_connections(well_info_ICON1, True)

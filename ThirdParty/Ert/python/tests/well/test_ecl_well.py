import datetime
from ert.ecl import EclGrid, EclFile, EclFileFlagEnum
from ert.test import ExtendedTestCase
from ert.util.ctime import CTime
from ert.well import WellInfo, WellConnection, WellTypeEnum, WellConnectionDirectionEnum, WellSegment


class EclWellTest(ExtendedTestCase):
    ALL_WELLS = ['E5H', 'G1H', 'M41', 'J41', 'D10H', 'P41', 'L41', 'M42', 'S41', 'S13H', 'Q12HT2', 'O41', 'L11H', 'Q21H',
                 'E6CH', 'D4Y1H', 'D4Y2H', 'I13Y1H', 'Q21AY1H', 'F5AH', 'O14Y4HT2', 'K24Y1H', 'K24Y3H', 'S21H', 'N11H',
                 'L23Y1H', 'D7AY1H', 'S21AHT2', 'H2AH', 'S21AHT3', 'L23Y6H', 'N11AH', 'N11BY1H', 'X23Y2H', 'N22Y3HT3',
                 'G1BY1HT2', 'Q21BY1H', 'Q21BY2T2', 'A08', 'A37', 'P24Y4H', 'A11', 'A33', 'P21BY1H', 'H6BY2H', 'M24AH',
                 'TOGIB2H', 'TOGIB3H', 'TOGIB4H', 'TOGIB5H', 'TOGIB6H', 'TAP21', 'D1AH', 'D1BYH', 'D2H', 'D2AYH', 'D4YH',
                 'D5AH', 'D6H', 'D7H', 'D7AYH', 'D8AH', 'D8BYH', 'E1AH', 'E1BYH', 'E3YH', 'E4H', 'E5AH', 'E6BH', 'E6DYH',
                 '31/2-T1', 'F1AH', 'F1BYH', 'F4AHT2', 'F5H', 'F5BYH', 'F6AH', 'F6BYH', '2B3H', 'G1AHT2', 'G1BYH', 'G3H',
                 'G3AYH', 'G4AH', 'G4BYH', 'G6HT2', 'G6AYH', '31/5-T1', 'H1AH', 'H2BYH', 'H2HT2', 'H3ABH', 'H3CYH', 'H4HT2',
                 'H5AHT2', 'H6AH', 'H6BYH', 'Z1H', 'Z2H', 'I11AH', 'I12AYH', 'I13YH', 'I14AHT2', 'I14BYH', 'I21AHT2',
                 'I21BYH', 'I22AH', 'I23AH', 'I32YH', 'J11AH', 'J12H', 'J13H', 'J13AYH', 'J14H', 'J14AYH', 'J21YH', 'J22YH',
                 'J22AYH', 'J23AH', 'J24HT2', 'J24AYH', 'K11YH', 'K12AH', 'K12BYH', 'K13AH', 'K14AYH', 'K21AYH', 'K22AH',
                 'K23AH', 'K24YH', 'L11AH', 'L11BYH', 'L12Y', 'L13H', 'L13AYH', 'L14YH', 'L21AYH', 'L22AYH', 'L23YH',
                 'L24AYH', 'M11H', 'M12H', 'M13YH', 'M14AHT2', 'M14BYH', 'M21H', 'M22H', 'M23AH', 'M24YH', 'N11BYH',
                 'N12HT2', 'N13H', 'N14AH', 'N21YH', 'N22YH', 'N23AYH', 'N24YH', 'O11H', 'O11AYH', 'O12AH', 'O13AH',
                 'O13BYH', 'O14YH', 'O21YH', 'O23YH', 'O26YH', 'P11YH', 'P11AYH', 'P12H', 'P13AHT2', 'P13BYH', 'P14AHT2',
                 'P14BYH', 'P21AYH', 'P21BYH', 'P22AYH', 'P23YH', 'P24YH', 'P24Y1H', 'Q11AHT2', 'Q11BYH', 'Q12AH', 'Q12BH',
                 'Q13AH', 'Q14H', 'Q14AYH', 'Q21AYH', 'Q21BH1', 'Q21BH2', 'S11HT3', 'S12H', 'S13AH', 'S14AH', 'S14BYH',
                 'S21BYH', 'S22AH', 'S23HT2', 'S23AYH', 'S24YH', 'S31H', 'X11AH', 'X11BYH', 'X12H', 'X12AH', 'X13YH',
                 'X14AYH', 'X21H', 'X22H', 'X22AYH', 'X23YH', 'X24AH', 'Y11AH', 'Y11BYH', 'Y12YH', 'Y13AH', 'Y13BYH',
                 'Y14AYH', 'Y21YH', 'Y22YH', 'Y23AHT2', 'Y23CYH', 'Y24AH']


    @classmethod
    def setUpClass(cls):
        EclWellTest.__well_info = None
        EclWellTest.__well_info_with_no_well_segments = None


    def getWellInfoWithNoWellSegments(self):
        """ @rtype: WellInfo """
        if EclWellTest.__well_info_with_no_well_segments is None:
            grid_path = self.createTestPath("Statoil/ECLIPSE/Troll/MSW/T07-4A-W2012-16-F3.EGRID")
            rst_path_1 = self.createTestPath("Statoil/ECLIPSE/Troll/MSW/T07-4A-W2012-16-F3.X0135")

            grid = EclGrid(grid_path)

            rst_file = EclFile(rst_path_1, EclFileFlagEnum.ECL_FILE_CLOSE_STREAM)

            EclWellTest.__well_info_with_no_well_segments = WellInfo(grid, rst_file, False)


        return EclWellTest.__well_info_with_no_well_segments

    def getWellInfo(self):
        """ @rtype: WellInfo """
        if EclWellTest.__well_info is None:
            grid_path = self.createTestPath("Statoil/ECLIPSE/Troll/MSW/T07-4A-W2012-16-F3.EGRID")
            rst_path_1 = self.createTestPath("Statoil/ECLIPSE/Troll/MSW/T07-4A-W2012-16-F3.X0135")

            grid = EclGrid(grid_path)

            rst_file = EclFile(rst_path_1, EclFileFlagEnum.ECL_FILE_CLOSE_STREAM)

            EclWellTest.__well_info = WellInfo(grid, rst_file)


        return EclWellTest.__well_info


    def test_construction(self):
        grid_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID")
        rst_path_1 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0011")
        rst_path_2 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0022")
        rst_path_3 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0035")
        rst_path_4 = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.X0061")

        grid = EclGrid(grid_path)

        def checkWellInfo(well_info, well_count, report_step_count):
            self.assertEqual(len(well_info), well_count)

            for index, well_time_line in enumerate(well_info):
                self.assertEqual(len(well_time_line), report_step_count[index])

        well_info = WellInfo(grid, rst_path_1)
        checkWellInfo(well_info, well_count=5, report_step_count=[1, 1, 1, 1, 1])

        well_info = WellInfo(grid, EclFile(rst_path_1))
        checkWellInfo(well_info, well_count=5, report_step_count=[1, 1, 1, 1, 1])

        well_info = WellInfo(grid, [rst_path_1, rst_path_2, rst_path_3])
        checkWellInfo(well_info, well_count=8, report_step_count=[3, 3, 3, 3, 3, 2, 2, 2])

        well_info = WellInfo(grid, [EclFile(rst_path_1), EclFile(rst_path_2), rst_path_3, EclFile(rst_path_4)])
        checkWellInfo(well_info, well_count=8, report_step_count=[4, 4, 4, 4, 4, 3, 3, 3])


        well_info = WellInfo(grid)
        well_info.addWellFile(rst_path_1, True)

        checkWellInfo(well_info, well_count=5, report_step_count=[1, 1, 1, 1, 1])

        well_info.addWellFile(EclFile(rst_path_2), True)
        checkWellInfo(well_info, well_count=8, report_step_count=[2, 2, 2, 2, 2, 1, 1, 1])

        well_info.addWellFile(EclFile(rst_path_3), True)
        checkWellInfo(well_info, well_count=8, report_step_count=[3, 3, 3, 3, 3, 2, 2, 2])

        well_info.addWellFile(rst_path_4, True)
        checkWellInfo(well_info, well_count=8, report_step_count=[4, 4, 4, 4, 4, 3, 3, 3])



    def test_well_type_enum(self):
        source_file_path = "libecl_well/include/ert/ecl_well/well_const.h"
        # The header file has duplicated symbols, so the simple test fails.
        # self.assertEnumIsFullyDefined(WellTypeEnum, "well_type_enum", source_file_path)


    def test_well_connection_direction_enum(self):
        source_file_path = "libecl_well/include/ert/ecl_well/well_conn.h"
        self.assertEnumIsFullyDefined(WellConnectionDirectionEnum, "well_conn_dir_enum", source_file_path)


    def test_well_info(self):
        well_info = self.getWellInfo()
        self.assertEqual(len(well_info), 222)

        all_well_names = well_info.allWellNames()

        self.assertEqual(len(well_info), len(all_well_names))
        self.assertEqual(EclWellTest.ALL_WELLS, all_well_names)

        for well_name in all_well_names:
            self.assertTrue(well_name in well_info)

        self.assertEqual(well_info[0], well_info[all_well_names[0]])
        self.assertEqual(well_info[1], well_info[all_well_names[1]])
        self.assertEqual(well_info[221], well_info[all_well_names[221]])

        self.assertNotEqual(well_info[0], well_info[1])

        well_time_lines = [wtl for wtl in well_info]
        self.assertEqual(len(well_time_lines), len(well_info))

        with self.assertRaises(IndexError):
            err = well_info[222]

        with self.assertRaises(KeyError):
            err = well_info["Well"]




    def test_well_time_line(self):
        well_info = self.getWellInfo()

        for well_time_line in well_info:
            self.assertEqual(len(well_time_line), 1)

        well_states = set()
        for well_name in EclWellTest.ALL_WELLS:
            well_time_line = well_info[well_name]
            well_states.add(well_time_line[0])

            with self.assertRaises(IndexError):
                err = well_time_line[1]

        self.assertEqual(len(well_states), len(EclWellTest.ALL_WELLS))

        # testing name and repr
        info = self.getWellInfo()
        wtl  = info['G6HT2']
        self.assertEqual('G6HT2', wtl.getName())
        rep = repr(wtl)
        print(rep)
        pfx = 'WellTimeLine('
        self.assertEqual(pfx, rep[:len(pfx)])

        # testing __getitem__ and its well state
        ws = wtl[0]
        self.assertTrue(ws.isOpen())
        self.assertEqual(ws.wellType(), WellTypeEnum.ECL_WELL_PRODUCER)
        self.assertTrue(ws.isMultiSegmentWell())
        pfx = 'WellState('
        self.assertEqual(pfx, repr(ws)[:len(pfx)])


    def test_well_state(self):
        well_info = self.getWellInfo()

        sim_time = CTime(datetime.date(2000, 1, 1))
        open_states = {True: 0, False: 0}
        msw_states = {True: 0, False: 0}
        well_types = {WellTypeEnum.ECL_WELL_ZERO: 0,
                      WellTypeEnum.ECL_WELL_PRODUCER: 0,
                      WellTypeEnum.ECL_WELL_GAS_INJECTOR: 0,
                      WellTypeEnum.ECL_WELL_OIL_INJECTOR: 0,
                      WellTypeEnum.ECL_WELL_WATER_INJECTOR: 0}

        segments = set()
        branches = set()
        connections = set()
        connections_count = 0

        for index, well_name in enumerate(EclWellTest.ALL_WELLS):
            well_time_line = well_info[well_name]
            well_state = well_time_line[0]

            self.assertEqual(well_state.name(), well_name)
            well_number = well_state.wellNumber()
            self.assertEqual(well_number, index)

            self.assertEqual(well_state.reportNumber(), 135)
            self.assertEqual(well_state.simulationTime(), sim_time)

            open_states[well_state.isOpen()] += 1
            msw_states[well_state.isMultiSegmentWell()] += 1

            well_types[well_state.wellType()] += 1

            self.assertTrue(well_state.hasGlobalConnections())
            global_connections = well_state.globalConnections()
            connections_count += len(global_connections)
            connections.update(global_connections)

            # branches.add(well_state.branches())
            # segments.add(well_state.segments())

        self.assertEqual(open_states[True], 53)
        self.assertEqual(open_states[False], 169)

        self.assertEqual(msw_states[True], 169)
        self.assertEqual(msw_states[False], 53)

        self.assertEqual(well_types[WellTypeEnum.ECL_WELL_ZERO], 0)
        self.assertEqual(well_types[WellTypeEnum.ECL_WELL_WATER_INJECTOR], 0)
        self.assertEqual(well_types[WellTypeEnum.ECL_WELL_OIL_INJECTOR], 0)
        self.assertEqual(well_types[WellTypeEnum.ECL_WELL_GAS_INJECTOR], 1)
        self.assertEqual(well_types[WellTypeEnum.ECL_WELL_PRODUCER], 221)

        self.assertEqual(len(connections), connections_count)



    def test_well_segments(self):
        well_info = self.getWellInfo()

        well_name = "X22AYH"
        well_time_line = well_info[well_name]
        well_state = well_time_line[0]

        segments = well_state.segments()

        branch_ids = [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
                      2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                      13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37,
                      38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55]

        outlet_ids = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
                      27, 1, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
                      52, 53, 54, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                      26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 45, 45, 47, 48, 49, 50, 51,
                      52, 53, 54, 55]

        depths = [1554.0, 1557.505, 1557.506, 1557.50540469, 1557.75140469, 1558.24240469, 1558.24240469, 1557.75340469,
                  1557.50740469, 1557.50658613, 1557.50628848, 1557.50584678, 1557.50384678, 1557.50484678, 1557.50402823,
                  1557.50380498, 1557.50454913, 1557.50425147, 1557.50325147, 1557.50273057, 1557.50473057, 1557.50373057,
                  1557.5038794, 1557.5048794, 1557.50398643, 1557.50406085, 1557.50606085, 1557.50670767, 1557.635, 1557.509,
                  1557.504, 1557.503, 1557.502, 1557.50242878, 1557.50542878, 1557.50505671, 1557.50602409, 1557.50602409,
                  1557.50402409, 1557.50485764, 1557.50563489, 1557.50543796, 1557.50643796, 1557.50843796, 1557.50799148,
                  1557.50709851, 1557.50665427, 1557.50865427, 1557.50943383, 1557.50543383, 1557.50443383, 1557.50495473,
                  1557.504285, 1557.50473149, 1557.50442009, 1557.505, 1557.506, 1557.50540469, 1557.75140469, 1558.24240469,
                  1558.24240469, 1557.75340469, 1557.50740469, 1557.50658613, 1557.50628848, 1557.50584678, 1557.50384678,
                  1557.50484678, 1557.50402823, 1557.50380498, 1557.50454913, 1557.50425147, 1557.50325147, 1557.50273057,
                  1557.50473057, 1557.50373057, 1557.5038794, 1557.5048794, 1557.50398643, 1557.50406085, 1557.50606085,
                  1557.50670767, 1557.635, 1557.509, 1557.504, 1557.503, 1557.502, 1557.50242878, 1557.50542878, 1557.50505671,
                  1557.50602409, 1557.50602409, 1557.50402409, 1557.50485764, 1557.50563489, 1557.50543796, 1557.50643796,
                  1557.50799148, 1557.50799148, 1557.50665427, 1557.50865427, 1557.50943383, 1557.50543383, 1557.50443383,
                  1557.50495473, 1557.504285, 1557.50473149, 1557.50442009]

        lengths = [1853.483, 525.634, 89.101, 148.227, 105.066, 70.981, 50.194, 71.215, 129.929, 141.712, 141.239, 108.247,
                   200.032, 116.122, 141.525, 141.983, 112.622, 72.694, 105.195, 149.555, 128.22, 83.537, 112.533, 155.336,
                   86.552, 71.427, 129.949, 92.347, 210.312, 58.0, 218.375, 111.0, 250.925, 76.233, 72.565, 127.481, 97.987,
                   75.648, 121.119, 129.137, 188.157, 109.433, 142.914, 84.9, 47.458, 57.507, 55.704, 211.767, 121.219,
                   143.23, 145.666, 146.044, 88.195, 72.891, 58.92, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
                   0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1,
                   0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1]

        total_lengths = [1853.483, 2379.117, 2468.218, 2616.445, 2721.511, 2792.492, 2842.686, 2913.901, 3043.83, 3185.542,
                         3326.781, 3435.028, 3635.06, 3751.182, 3892.707, 4034.69, 4147.312, 4220.006, 4325.201, 4474.756,
                         4602.976, 4686.513, 4799.046, 4954.382, 5040.934, 5112.361, 5242.31, 5334.657, 2063.795, 2121.795,
                         2340.17, 2451.17, 2702.095, 2778.328, 2850.893, 2978.374, 3076.361, 3152.009, 3273.128, 3402.265,
                         3590.422, 3699.855, 3842.769, 3927.669, 3975.127, 4032.634, 4088.338, 4300.105, 4421.324, 4564.554,
                         4710.22, 4856.264, 4944.459, 5017.35, 5076.27, 2379.217, 2468.318, 2616.545, 2721.611, 2792.592,
                         2842.786, 2914.001, 3043.93, 3185.642, 3326.881, 3435.128, 3635.16, 3751.282, 3892.807, 4034.79,
                         4147.412, 4220.106, 4325.301, 4474.856, 4603.076, 4686.613, 4799.146, 4954.482, 5041.034, 5112.461,
                         5242.41, 5334.757, 2063.895, 2121.895, 2340.27, 2451.27, 2702.195, 2778.428, 2850.993, 2978.474,
                         3076.461, 3152.109, 3273.228, 3402.365, 3590.522, 3699.955, 3842.869, 3975.227, 3975.227, 4088.438,
                         4300.205, 4421.424, 4564.654, 4710.32, 4856.364, 4944.559, 5017.45, 5076.37]

        link_count = 0
        main_stem = {True: 0, False: 0}
        nearest_wellhead = {True: 0, False: 0}
        for index, segment in enumerate(segments):
            assert isinstance(segment, WellSegment)
            self.assertEqual(segment.id(), index + 1)
            link_count += segment.linkCount()
            self.assertEqual(segment.branchId(), branch_ids[index])
            self.assertEqual(segment.outletId(), outlet_ids[index])

            self.assertTrue(segment.isActive())
            main_stem[segment.isMainStem()] += 1
            nearest_wellhead[segment.isNearestWellHead()] += 1

            self.assertFloatEqual(segment.depth(), depths[index])
            self.assertFloatEqual(segment.length(), lengths[index])
            self.assertFloatEqual(segment.totalLength(), total_lengths[index])

            if index == 0:
                self.assertEqual(segment.diameter(), 0.0)
            else:
                self.assertEqual(segment.diameter(), 0.159)


        self.assertEqual(main_stem[True], 28)
        self.assertEqual(main_stem[False], 80)

        self.assertEqual(nearest_wellhead[True], 1)
        self.assertEqual(nearest_wellhead[False], 107)

        self.assertEqual(link_count, 53)



    def test_well_connections(self):
        well_info = self.getWellInfo()

        well_name = "H6BY2H"
        well_connections_ijk = [(33, 157, 9), (32, 157, 9), (32, 157, 51), (32, 157, 52),
                                (32, 157, 53), (32, 157, 54), (32, 157, 55), (32, 157, 56),
                                (32, 157, 57), (31, 157, 57), (31, 158, 57), (31, 158, 56),
                                (30, 158, 56), (29, 158, 56), (29, 157, 56), (28, 157, 56),
                                (28, 158, 56), (28, 158, 55), (27, 158, 55), (27, 158, 54),
                                (26, 158, 54), (26, 158, 53), (25, 158, 53), (24, 158, 53),
                                (23, 158, 53), (23, 158, 54), (22, 158, 54), (21, 158, 54),
                                (20, 158, 54), (20, 158, 55), (19, 158, 55), (19, 158, 54),
                                (18, 158, 54), (17, 158, 54), (16, 158, 54), (16, 158, 55),
                                (15, 158, 55), (15, 158, 54), (15, 158, 53), (14, 158, 54),
                                (13, 158, 54), (13, 158, 55), (12, 158, 55), (12, 157, 55)]

        well_connection_factors = [3022.45092773, 171.561004639, 237.263000488, 135.57800293, 177.925994873, 289.058990479,
                                   1081.09997559, 1575.79101562, 3679.54907227, 2865.51489258, 3999.2199707, 14205.3300781,
                                   1864.43005371, 1296.47302246, 3.40599989891, 2012.7199707, 2656.25390625, 4144.21923828,
                                   6.22700023651, 96.4029998779, 0.144999995828, 80.81199646, 114.416999817, 97.5159988403,
                                   26.8530006409, 0.12800000608, 34238.15625, 34493.7070312, 2618.16894531, 31999.1992188,
                                   27874.6191406, 7343.23681641, 35418.1679688, 34612.6523438, 3486.13500977, 15446.3691406,
                                   65.4499969482, 8687.91113281, 13238.8037109, 5644.90380859, 7499.49707031, 12863.5292969,
                                   12277.4716797, 19404.5488281]

        well_time_line = well_info[well_name]
        well_state = well_time_line[0]

        self.assertFalse(well_state.isMultiSegmentWell())

        self.assertTrue(well_state.hasGlobalConnections())
        global_connections = well_state.globalConnections()

        for index, connection in enumerate(global_connections):
            assert isinstance(connection, WellConnection)
            self.assertTrue(connection.isOpen())
            self.assertEqual(connection.ijk(), well_connections_ijk[index])
            self.assertFalse(connection.isMultiSegmentWell())
            self.assertEqual(connection.segmentId(), -999)
            self.assertFalse(connection.isFractureConnection())
            self.assertTrue(connection.isMatrixConnection())
            self.assertFloatEqual(connection.connectionFactor(), well_connection_factors[index])
            self.assertEqual(connection.direction(), WellConnectionDirectionEnum.well_conn_dirX)

        self.assertNotEqual(global_connections[0], global_connections[1])
        self.assertEqual(global_connections[0], global_connections[0])


    def test_well_connections_msw(self):
        well_info = self.getWellInfo()

        well_name = "X22AYH"
        well_time_line = well_info[well_name]
        well_state = well_time_line[0]

        segment_ids = [56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                       80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
                       103, 104, 105, 106, 107, 108]

        self.assertTrue(well_state.isMultiSegmentWell())

        self.assertTrue(well_state.hasGlobalConnections())
        global_connections = well_state.globalConnections()

        self.assertEqual(len(global_connections), len(segment_ids))

        for index, connection in enumerate(global_connections):
            assert isinstance(connection, WellConnection)
            self.assertTrue(connection.isOpen())
            self.assertTrue(connection.isMultiSegmentWell())
            self.assertEqual(connection.segmentId(), segment_ids[index])
            self.assertFalse(connection.isFractureConnection())
            self.assertTrue(connection.isMatrixConnection())
            self.assertEqual(connection.direction(), WellConnectionDirectionEnum.well_conn_dirY)


    def test_well_connections_msw_do_not_load_segments(self):
        well_info = self.getWellInfoWithNoWellSegments()

        well_name = "X22AYH"
        well_time_line = well_info[well_name]
        well_state = well_time_line[0]

        segment_ids = [56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                       80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
                       103, 104, 105, 106, 107, 108]

        self.assertTrue(well_state.isMultiSegmentWell())
        self.assertFalse(well_state.hasSegmentData())

        self.assertTrue(well_state.hasGlobalConnections())
        global_connections = well_state.globalConnections()

        self.assertEqual(len(global_connections), len(segment_ids))

        for index, connection in enumerate(global_connections):
            assert isinstance(connection, WellConnection)
            self.assertTrue(connection.isOpen())
            self.assertTrue(connection.isMultiSegmentWell())
            self.assertEqual(connection.segmentId(), segment_ids[index])
            self.assertFalse(connection.isFractureConnection())
            self.assertTrue(connection.isMatrixConnection())
            self.assertEqual(connection.direction(), WellConnectionDirectionEnum.well_conn_dirY)

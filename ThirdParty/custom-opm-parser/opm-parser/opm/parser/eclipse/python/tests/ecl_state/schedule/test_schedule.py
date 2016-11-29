import datetime
import os.path
from ert.test import TestAreaContext

from opm_test import OpmTest
from opm.parser import Parser
from opm.ecl_state.schedule import Schedule
from opm.ecl_state.grid import EclipseGrid



class ScheduleTest(OpmTest):
    def setUp(self):
        pass

    def test_parse(self):
        p = Parser()

        # This is slightly awkward; since only the deck based
        # constructor is wrapped in the grid case we need to different
        # input files to instantiate the Schedule and EclipseGrid
        # instances respectively.
        grid_file = self.createPath( "integration_tests/GRID/CORNERPOINT_ACTNUM.DATA" )
        grid_deck = p.parseFile( grid_file )
        grid = EclipseGrid(grid_deck)

        sched_file = self.createPath("integration_tests/SCHEDULE/SCHEDULE1")
        sched_deck = p.parseFile( sched_file )
        s = Schedule( grid , sched_deck )

        end = s.endTime()
        self.assertEqual( datetime.datetime( end.year , end.month , end.day) , datetime.datetime( 2008 , 8 , 10 ))
        #self.assertEqual( s.endTime( ) , datetime.datetime( 2008 , 8 , 10 ))

        start = s.startTime()
        self.assertEqual( datetime.datetime( start.year , start.month , start.day) , datetime.datetime( 2007 , 5 , 10 ))


import unittest

from opm.io.parser import Parser
from opm.io.ecl_state import EclipseState
from opm.io.schedule import Schedule
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path

def flowing(connection):
    return connection.state == 'OPEN'

def closed(connection):
    return connection.state == 'SHUT'


class TestWells(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        deck = Parser().parse(test_path('spe3/SPE3CASE1.DATA'))
        print("Creating state")
        cls.state = EclipseState(deck)
        print("State OK")
        cls.sch = Schedule(deck, cls.state)
        cls.timesteps = cls.sch.timesteps

    def test_connection_pos(self):
        wells = self.sch.get_wells(0)
        p00 = wells[0].connections()[0].pos
        p01 = wells[0].connections()[1].pos
        p10 = wells[1].connections()[0].pos
        p11 = wells[1].connections()[1].pos
        self.assertEqual(p00, (6,6,2))
        self.assertEqual(p01, (6,6,3))
        self.assertEqual(p10, (0,0,0))
        self.assertEqual(p11, (0,0,1))

    def test_connection_state(self):
        for timestep,_ in enumerate(self.timesteps):
            for well in self.sch.get_wells(timestep):
                for connection in well.connections():
                    self.assertEqual("OPEN", connection.state)

    def test_filters(self):
        connections = self.sch.get_wells(0)[0].connections()
        self.assertEqual(len(list(filter(flowing, connections))), 2)
        self.assertEqual(len(list(filter(closed, connections))), 0)

    def test_direction(self):
       for timestep,_ in enumerate(self.timesteps):
           for well in self.sch.get_wells(timestep):
                for connection in well.connections():
                    self.assertEqual(connection.direction, 'Z')

    def test_attached_to_segment(self):
        for timestep,_ in enumerate(self.timesteps):
            for well in self.sch.get_wells(timestep):
                for connection in well.connections():
                    self.assertFalse(connection.attached_to_segment)


if __name__ == "__main__":
    unittest.main()

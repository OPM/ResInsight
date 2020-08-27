import unittest
import opm
from opm.io.parser import Parser
from opm.io.ecl_state import EclipseState
from opm.io.schedule import Schedule

try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path

def injector(well):
    return well.isinjector()

def producer(well):
    return well.isproducer()

def defined(timestep):
    def fn(well): return well.isdefined(timestep)
    return fn

def open_at_1(well):
    return well.status() == 'OPEN'

def closed(well):
    return well.status() == 'SHUT'


class TestWells(unittest.TestCase):


    @classmethod
    def setUpClass(cls):
        deck = Parser().parse(test_path('spe3/SPE3CASE1.DATA'))
        state = EclipseState(deck)
        cls.sch = Schedule( deck, state )
        cls.timesteps = cls.sch.timesteps

    def inje(self, wells):
        return next(iter(filter(injector, wells)))

    def prod(self, wells):
        return next(iter(filter(producer, wells)))


    def testWellPos0(self):
        wells = self.sch.get_wells(0)
        well = wells[0]
        print( well.pos() )
        i, j, refdepth = well.pos()

        self.assertEqual(6, i)
        self.assertEqual(6, j)
        self.assertEqual(2247.9, refdepth)

    def testWellStatus(self):
        wells = self.sch.get_wells(0)
        for well in wells:
            self.assertEqual("OPEN", well.status())

    def testGroupName(self):
        wells = self.sch.get_wells(0)
        for well in wells:
            self.assertEqual("G1", well.group())

    def testPreferredPhase(self):
        wells = self.sch.get_wells(0)
        for well in wells:
            self.assertTrue("GAS", well.preferred_phase)

    def testGuideRate(self):
        wells = self.sch.get_wells(1)
        inje, prod = self.inje(wells), self.prod(wells)
        self.assertEqual(-1.0, inje.guide_rate())
        self.assertEqual(-1.0, prod.guide_rate())

        wells = self.sch.get_wells(14)
        inje, prod = self.inje(wells), self.prod(wells)
        self.assertEqual(-1.0, inje.guide_rate())
        self.assertEqual(-1.0, prod.guide_rate())

    def testGroupControl(self):
        wells = self.sch.get_wells(1)
        inje, prod = self.inje(wells), self.prod(wells)
        self.assertTrue(inje.available_gctrl())
        self.assertTrue(prod.available_gctrl())

        wells = self.sch.get_wells(14)
        inje, prod = self.inje(wells), self.prod(wells)
        self.assertTrue(inje.available_gctrl())
        self.assertTrue(prod.available_gctrl())

    def testWellDefinedFilter(self):

        defined0 = list(filter(defined(0), self.sch.get_wells(0) ))
        defined1 = list(filter(defined(1), self.sch.get_wells(1) ))
        self.assertEqual(len(list(defined0)), 2)
        self.assertEqual(len(list(defined1)), 2)

    def testWellProdInjeFilter(self):
        inje = list(filter(injector, self.sch.get_wells(0) ))
        prod = list(filter(producer, self.sch.get_wells(0) ))

        self.assertEqual(len(inje), 1)
        self.assertEqual(len(prod), 1)

        self.assertEqual(inje[0].name, "INJ")
        self.assertEqual(prod[0].name, "PROD")

    def testOpenFilter(self):
        wells = self.sch.get_wells(1)

        flowing_list = list(filter(open_at_1, wells))
        closed_list  = list(filter(lambda well: not open_at_1(well), wells))

        self.assertEqual(2, len(flowing_list))
        self.assertEqual(0, len(closed_list))

        flowing1_list = list(filter(lambda well: not closed(well), wells))
        closed1_list  = list(filter(closed, wells))
        self.assertListEqual(list(closed_list), list(closed1_list))


    def testCompletions(self):
        num_steps = len( self.sch.timesteps )
        w0 = self.sch.get_wells(num_steps - 1)[0]
        c0,c1 = w0.connections()

        self.assertEqual((6,6,2), c0.pos)
        self.assertEqual((6,6,3), c1.pos)


if __name__ == "__main__":
    unittest.main()

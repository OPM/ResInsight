import unittest
import os.path
import sys

import numpy as np

from opm.io.parser import Parser
from opm.io.parser import ParseContext
from opm.io.deck import DeckKeyword
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path


unit_foot = 0.3048 #meters

class TestParser(unittest.TestCase):

    REGIONDATA = """
START             -- 0
10 MAI 2007 /
RUNSPEC
FIELD
DIMENS
2 2 1 /
GRID
DX
4*0.25 /
DY
4*0.25 /
DZ
4*0.25 /
TOPS
4*0.25 /
REGIONS
OPERNUM
3 3 1 2 /
FIPNUM
1 1 2 3 /
"""

    def setUp(self):
        self.spe3fn = test_path('spe3/SPE3CASE1.DATA')
        self.norne_fname = test_path('../examples/data/norne/NORNE_ATW2013.DATA')

    def test_create(self):
        parser = Parser()
        deck = parser.parse(self.spe3fn)
        active_unit_system = deck.active_unit_system()
        default_unit_system = deck.default_unit_system()
        self.assertEqual(active_unit_system.name, "Field")

        context = ParseContext()
        deck = parser.parse(self.spe3fn, context)

        with open(self.spe3fn) as f:
            string = f.read()
        deck = parser.parse_string(string)
        deck = parser.parse_string(string, context)

    def test_deck_kw_records(self):
        parser = Parser()
        deck = parser.parse_string(self.REGIONDATA)
        active_unit_system = deck.active_unit_system()
        default_unit_system = deck.default_unit_system()
        self.assertEqual(active_unit_system.name, "Field")

        with self.assertRaises(ValueError):
            kw = parser["NOT_A_VALID_KEYWORD"]

        field = parser["FIELD"]
        assert(field.name == "FIELD")

        dkw_field = DeckKeyword(field)
        assert(dkw_field.name == "FIELD")

        DeckKeyword(parser["AQUCWFAC"], [[]], active_unit_system, default_unit_system)

        with self.assertRaises(TypeError):
            dkw_wrong =  DeckKeyword(parser["AQUCWFAC"], [22.2, 0.25], active_unit_system, default_unit_system)

        dkw_aqannc = DeckKeyword(parser["AQANNC"], [[12, 1, 2, 3, 0.89], [13, 4, 5, 6, 0.625]], active_unit_system, default_unit_system)
        assert( len(dkw_aqannc[0]) == 5 )
        assert( dkw_aqannc[0][2].get_int(0) == 2 )
        assert( dkw_aqannc[1][1].get_int(0) == 4 )
        with self.assertRaises(ValueError):
            value = dkw_aqannc[1][1].get_raw(0)
        with self.assertRaises(ValueError):
            value = dkw_aqannc[1][1].get_SI(0)
        assert( dkw_aqannc[1][4].get_raw(0) == 0.625 )
        self.assertAlmostEqual( dkw_aqannc[1][4].get_SI(0), 0.625 * unit_foot**2 )
        assert( dkw_aqannc[1][4].get_raw_data_list() == [0.625] )
        self.assertAlmostEqual( dkw_aqannc[1][4].get_SI_data_list()[0], 0.625 * unit_foot**2 )
        with self.assertRaises(ValueError):
            value = dkw_aqannc[1][4].get_int(0)

        dkw_aqantrc = DeckKeyword(parser["AQANTRC"], [[12, "ABC", 8]], active_unit_system, default_unit_system)
        assert( dkw_aqantrc[0][1].get_str(0) == "ABC" )
        assert( dkw_aqantrc[0][2].get_raw(0) == 8.0 )

        dkw1 = DeckKeyword(parser["AQUCWFAC"], [["*", 0.25]], active_unit_system, default_unit_system)
        assert( dkw1[0][0].get_raw(0) == 0.0 )
        assert( dkw1[0][1].get_raw(0) == 0.25 )

        dkw2 = DeckKeyword(parser["AQUCWFAC"], [[0.25, "*"]], active_unit_system, default_unit_system)
        assert( dkw2[0][0].get_raw(0) == 0.25 )
        assert( dkw2[0][1].get_raw(0) == 1.0 )

        dkw3 = DeckKeyword(parser["AQUCWFAC"], [[0.50]], active_unit_system, default_unit_system)
        assert( dkw3[0][0].get_raw(0) == 0.50 )
        assert( dkw3[0][1].get_raw(0) == 1.0 )

        dkw4 = DeckKeyword(parser["CBMOPTS"], [["3*", "A", "B", "C", "2*", 0.375]], active_unit_system, default_unit_system)
        assert( dkw4[0][0].get_str(0) == "TIMEDEP" )
        assert( dkw4[0][2].get_str(0) == "NOKRMIX" )
        assert( dkw4[0][3].get_str(0) == "A" )
        assert( dkw4[0][6].get_str(0) == "PMPVK" )
        assert( dkw4[0][8].get_raw(0) == 0.375 )
        with self.assertRaises(TypeError):
            dkw4[0][8].get_data_list()

        with self.assertRaises(TypeError):
            DeckKeyword(parser["CBMOPTS"], [["3*", "A", "B", "C", "R2*", 0.77]], active_unit_system, default_unit_system)

        with self.assertRaises(TypeError):
            DeckKeyword(parser["CBMOPTS"], [["3*", "A", "B", "C", "2.2*", 0.77]], active_unit_system, default_unit_system)

        dkw5 = DeckKeyword(parser["AQUCWFAC"], [["2*5.5"]], active_unit_system, default_unit_system)
        assert( dkw5[0][0].get_raw(0) == 5.5 )
        assert( dkw5[0][1].get_raw(0) == 5.5 )

        with self.assertRaises(ValueError):
            raise DeckKeyword(parser["AQANTRC"], [["1*2.2", "ABC", 8]], active_unit_system, default_unit_system)


    def test_deck_kw_vector(self):
        parser = Parser()
        deck = parser.parse_string(self.REGIONDATA)
        active_unit_system = deck.active_unit_system()
        default_unit_system = deck.default_unit_system()
        self.assertEqual(active_unit_system.name, "Field")

        int_array = np.array([0, 1, 2, 3])
        hbnum_kw = DeckKeyword( parser["HBNUM"], int_array)
        assert( np.array_equal(hbnum_kw.get_int_array(), int_array) )

        raw_array = np.array([1.1, 2.2, 3.3])
        zcorn_kw = DeckKeyword( parser["ZCORN"], raw_array, active_unit_system, default_unit_system)
        assert( np.array_equal(zcorn_kw.get_raw_array(), raw_array) )
        si_array = zcorn_kw.get_SI_array()
        self.assertAlmostEqual( si_array[0], 1.1 * unit_foot )
        self.assertAlmostEqual( si_array[2], 3.3 * unit_foot )

        assert( not( "ZCORN" in deck ) )
        deck.add( zcorn_kw )
        assert( "ZCORN" in deck )
        
    

if __name__ == "__main__":
    unittest.main()


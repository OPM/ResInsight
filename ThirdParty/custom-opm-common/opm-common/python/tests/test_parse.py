import unittest
import opm.io
import os.path
import sys

from opm.io.parser import Parser, ParseContext
from opm.io.ecl_state import EclipseState
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path

class TestParse(unittest.TestCase):

    REGIONDATA = """
START             -- 0
10 MAI 2007 /
RUNSPEC

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

    def test_parse(self):
        deck = Parser().parse(self.spe3fn)
        state = EclipseState(deck)
        self.assertEqual('SPE 3 - CASE 1', state.title)

    def test_parse_with_recovery(self):
        recovery = [("PARSE_RANDOM_SLASH", opm.io.action.ignore)]
        parse_context = ParseContext( recovery )
        deck = Parser().parse(self.spe3fn, parse_context)
        state = EclipseState(deck)

    def test_parse_with_multiple_recoveries(self):
        recoveries = [ ("PARSE_RANDOM_SLASH", opm.io.action.ignore),
                       ("FOO", opm.io.action.warn),
                       ("PARSE_RANDOM_TEXT", opm.io.action.throw) ]

        parse_context = ParseContext(recoveries)
        deck = Parser().parse(self.spe3fn, parse_context)
        state = EclipseState(deck)

    def test_throw_on_invalid_recovery(self):
        recoveries = [ ("PARSE_RANDOM_SLASH", 3.14 ) ]

        with self.assertRaises(TypeError):
            parse_context = ParseContext(recoveries)
            deck = Parser().parse(self.spe3fn, parse_context)

        with self.assertRaises(TypeError):
            parse_context = ParseContext("PARSE_RANDOM_SLASH")
            deck = Parser().parse(self.spe3fn, parse_context)

    def test_data(self):
        pass
        #regtest = opm.parse(self.REGIONDATA)
        #self.assertEqual([3,3,1,2], regtest.props()['OPERNUM'])

    def test_parse_norne(self):
         parse_context = ParseContext( [('PARSE_RANDOM_SLASH', opm.io.action.ignore)] )
         deck = Parser().parse(self.norne_fname, parse_context)
         es = EclipseState( deck )

         self.assertEqual(46, es.grid().nx)
         self.assertEqual(112, es.grid().ny)
         self.assertEqual(22, es.grid().nz)

if __name__ == "__main__":
    unittest.main()


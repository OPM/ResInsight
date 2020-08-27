import unittest

from opm.io.parser import Parser

class TestParse(unittest.TestCase):

    DECK_STRING = """
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
        self.deck = Parser().parse_string(self.DECK_STRING)

    def test_deck_in(self):
        map(lambda kw: self.assertIn(kw, self.deck), [
            'START', 'RUNSPEC', 'DIMENS',
            'GRID', 'TOPS', 'REGIONS',
            'OPERNUM', 'FIPNUM'
        ])
        map(lambda kw: self.assertNotIn(kw, self.deck), [
            'PPCWMAX', 'APIGROUP', 'NOWARN', 'WARN', 'TBLKFAI4', 'WINJMULT'
        ])

    def test_deck_str(self):
        self.assertEqual(
            'DIMENS 2 2 1 /'.split(),
            str(self.deck['DIMENS']).split()
        )
        self.assertEqual(
            'DX 0.25 0.25 0.25 0.25 /'.split(),
            str(self.deck['DX']).split()
        )
        self.assertEqual(
            str(  Parser().parse_string('RUNSPEC\n\nDX\n4*0.5 /')  ).split(),
            'RUNSPEC DX 0.5 0.5 0.5 0.5 /'.split()
        )

    def test_deck_keyword(self):
        for kw in self.deck:
            self.assertTrue(len(kw.name) > 0)
            for rec in kw:
                self.assertTrue( len(rec) > 0 )
                for item in rec:
                    self.assertTrue(len(item) > 0)

    def test_deck_FIPNUM(self):
        self.assertIn('FIPNUM', self.deck)
        self.assertEqual(len(self.deck['FIPNUM']), 1)
        self.assertEqual(len(self.deck['FIPNUM'][0]), 1)
        self.assertEqual(len(self.deck['FIPNUM'][0][0].get_data_list()), 4)


if __name__ == "__main__":
    unittest.main()


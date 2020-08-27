import datetime
import unittest

import opm.io.sim

class TestSummaryState(unittest.TestCase):

    def setUp(self):
        pass

    def test_create(self):
        st = opm.io.sim.SummaryState(datetime.datetime.now())
        st.update("FOPT", 100)
        self.assertEqual(st["FOPT"], 100)
        self.assertTrue("FOPT" in st)
        self.assertFalse("FWPR" in st)

        with self.assertRaises(IndexError):
            x = st["FWPR"]

        st.update_well_var("OP1", "WOPR", 100)
        st.update_well_var("OP2", "WOPR", 200)
        st.update_well_var("OP3", "WOPR", 300)
        self.assertEqual(st.well_var("OP1", "WOPR"), 100)

        st.update_group_var("G1", "GOPR", 100)
        st.update_group_var("G2", "GOPR", 200)
        st.update_group_var("G3", "GOPR", 300)
        self.assertEqual(st.group_var("G3", "GOPR"), 300)

        self.assertTrue(st.has_group_var("G1", "GOPR"))
        self.assertFalse(st.has_well_var("OP1", "GOPR"))

        groups = st.groups
        self.assertEqual(len(groups), 3)
        self.assertTrue( "G1" in groups )
        self.assertTrue( "G2" in groups )
        self.assertTrue( "G3" in groups )

        wells = st.wells
        self.assertEqual(len(wells), 3)
        self.assertTrue( "OP1" in wells )
        self.assertTrue( "OP2" in wells )
        self.assertTrue( "OP3" in wells )

        el = st.elapsed()

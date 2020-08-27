import unittest
import sys
import numpy as np
import datetime

from opm.io.ecl import ESmry
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path



class TestEclFile(unittest.TestCase):

    def test_base_runs(self):

        with self.assertRaises(ValueError):
            ESmry("/file/that/does_not_exists")

        smry1 = ESmry(test_path("data/SPE1CASE1.SMSPEC"))

        self.assertEqual(len(smry1), 67)
        self.assertEqual(smry1.start_date, datetime.datetime(2015,1,1) )
        self.assertEqual(smry1.end_date, datetime.datetime(2020,4,29) )

        self.assertTrue("TIME" in smry1)
        self.assertTrue("BPR:10,10,3" in smry1)
        self.assertFalse("XXXX" in smry1)

        with self.assertRaises(ValueError):
            test = smry1["XXX"]

        time1a = smry1["TIME"]

        self.assertEqual(len(time1a), len(smry1))

        time1b = smry1["TIME", True]

        self.assertEqual(len(time1b), 64)


    def test_restart_runs(self):

        base_smry = ESmry(test_path("data/SPE1CASE1.SMSPEC"))
        self.assertEqual(base_smry.start_date, datetime.datetime(2015,1,1) )
        time0 = base_smry["TIME"]
        self.assertEqual(time0[0], 1.0)

        rst_smry1 = ESmry(test_path("data/SPE1CASE1_RST60.SMSPEC"))
        time1 = rst_smry1["TIME"]
        gor1 = rst_smry1["WGOR:PROD"]

        self.assertEqual(rst_smry1.start_date, datetime.datetime(2015,1,1) )
        self.assertEqual(len(rst_smry1), 60)
        self.assertEqual(time1[0], 1856.0)
        self.assertEqual(len(time1), 60)
        self.assertEqual(len(gor1), 60)

        rst_smry2 = ESmry(test_path("data/SPE1CASE1_RST60.SMSPEC"), load_base_run = True)
        time2 = rst_smry2["TIME"]
        gor2 = rst_smry2["WGOR:PROD"]

        self.assertEqual(rst_smry2.start_date, datetime.datetime(2015,1,1) )
        self.assertEqual(len(rst_smry2), 123)

        self.assertEqual(time2[0], 1.0)
        self.assertEqual(len(rst_smry2), 123)
        self.assertEqual(len(time2), 123)
        self.assertEqual(len(gor2), 123)


    def test_keylist(self):

        ref_key_list = ["BPR:1,1,1", "BPR:10,10,3", "FGOR", "FOPR", "TIME", "WBHP:INJ", "WBHP:PROD",
                        "WGIR:INJ", "WGIR:PROD", "WGIT:INJ", "WGIT:PROD", "WGOR:PROD", "WGPR:INJ",
                        "WGPR:PROD", "WGPT:INJ", "WGPT:PROD", "WOIR:INJ", "WOIR:PROD", "WOIT:INJ",
                        "WOIT:PROD", "WOPR:INJ", "WOPR:PROD", "WOPT:INJ", "WOPT:PROD", "WWIR:INJ",
                        "WWIR:PROD", "WWIT:INJ", "WWIT:PROD", "WWPR:INJ", "WWPR:PROD", "WWPT:INJ",
                        "WWPT:PROD"]

        ref_keys_pattern = ["WGPR:INJ", "WGPR:PROD", "WOPR:INJ", "WOPR:PROD", "WWPR:INJ", "WWPR:PROD"]

        smry1 = ESmry(test_path("data/SPE1CASE1.SMSPEC"))

        list_of_keys = smry1.keys()
        self.assertEqual(len(list_of_keys), len(ref_key_list))

        for key, ref_key in zip(list_of_keys, ref_key_list):
            self.assertEqual(key, ref_key)

        for key in list_of_keys:
            data = smry1[key]
            self.assertEqual(len(smry1), len(data))

        list_of_keys2 = smry1.keys("W?PR:*")

        self.assertEqual(len(list_of_keys2), len(ref_keys_pattern))

        for key, ref in zip(list_of_keys2, ref_keys_pattern):
            self.assertEqual(key, ref)

if __name__ == "__main__":

    unittest.main()


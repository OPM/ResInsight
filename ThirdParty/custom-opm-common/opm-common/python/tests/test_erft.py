import unittest
import sys
import numpy as np
import datetime

from opm.io.ecl import ERft, eclArrType
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path



class TestEclFile(unittest.TestCase):

    def test_content(self):

        refContent = [ ('PROD', (2015, 1, 1), 0.0),
                       ('INJ', (2015, 1, 1), 0.0),
                       ('A-1H', (2015, 9, 1), 243.0),
                       ('B-2H', (2016, 5, 31), 516.0),
                       ('PROD', (2017, 7, 31), 942.0) ]


        with self.assertRaises(ValueError):
            ERft("/file/that/does_not_exists")

        rft1 = ERft(test_path("data/SPE1CASE1.RFT"))
        self.assertEqual(len(rft1), 5)

        self.assertTrue( ("PROD", 2015, 1, 1) in rft1 )
        self.assertFalse( ("XXX", 2015, 1, 1) in rft1 )

        self.assertTrue( ("PRESSURE", "PROD", 2015, 1, 1) in rft1 )
        self.assertFalse( ("XXX", "PROD", 2015, 1, 1) in rft1 )

        self.assertTrue( ("PRESSURE", 0) in rft1 )
        self.assertTrue( ("PRESSURE", 1) in rft1 )

        self.assertFalse( ("XXX", 0) in rft1 )

        with self.assertRaises(ValueError):
            self.assertTrue( ("PRESSURE", "XX", "XX") in rft1 )

        rftlist = rft1.list_of_rfts

        self.assertEqual(len(rftlist), 5)

        for n, rftdata in enumerate(rftlist):
            self.assertEqual(rftdata[0], refContent[n][0])
            self.assertEqual(rftdata[1], refContent[n][1])
            self.assertEqual(rftdata[2], refContent[n][2])


    def test_list_of_rfts(self):

        ref_rftlist = []
        ref_rftlist.append(('PROD', (2015, 1, 1), 0.0))
        ref_rftlist.append(('INJ', (2015, 1, 1), 0.0))
        ref_rftlist.append(('A-1H', (2015, 9, 1), 243.0))
        ref_rftlist.append(('B-2H', (2016, 5, 31), 516.0))
        ref_rftlist.append(('PROD', (2017, 7, 31), 942.0))

        rft1 = ERft(test_path("data/SPE1CASE1.RFT"))

        for n, element in enumerate(rft1.list_of_rfts):
            self.assertEqual(element, ref_rftlist[n])


    def test_list_of_array(self):

        refArrList = ["TIME", "DATE", "WELLETC", "CONIPOS", "CONJPOS", "CONKPOS", "HOSTGRID", "DEPTH", "PRESSURE",
                      "SWAT", "SGAS"]

        rft1 = ERft(test_path("data/SPE1CASE1.RFT"))

        rft_date = (2016, 5, 31)
        arrList1 = rft1.arrays( "B-2H", rft_date )
        arrList2 = rft1.arrays( 3 )

        self.assertEqual(len(arrList1), len(refArrList))

        for n, (name, arrType, arrSize) in enumerate(arrList1):

            self.assertEqual(name, refArrList[n])

            if arrType != eclArrType.MESS:
                array = rft1[name, "B-2H", 2016, 5, 31]
                self.assertEqual(len(array), arrSize)

            if arrType == eclArrType.INTE:
                self.assertEqual(array.dtype, "int32")
            elif arrType == eclArrType.REAL:
                self.assertEqual(array.dtype, "float32")
            elif arrType == eclArrType.DOUB:
                self.assertEqual(array.dtype, "float64")
            elif arrType == eclArrType.LOGI:
                self.assertEqual(array.dtype, "bool")
            elif arrType == eclArrType.CHAR:
                self.assertTrue(array.dtype.kind in {'U', 'S'})


        self.assertEqual(len(arrList2), len(refArrList))

        for n, (name, arrType, arrSize) in enumerate(arrList2):

            self.assertEqual(name, refArrList[n])

            if arrType != eclArrType.MESS:
                array = rft1[name, "B-2H", 2016, 5, 31 ]
                self.assertEqual(len(array), arrSize)

            if arrType == eclArrType.INTE:
                self.assertEqual(array.dtype, "int32")
            elif arrType == eclArrType.REAL:
                self.assertEqual(array.dtype, "float32")
            elif arrType == eclArrType.DOUB:
                self.assertEqual(array.dtype, "float64")
            elif arrType == eclArrType.LOGI:
                self.assertEqual(array.dtype, "bool")
            elif arrType == eclArrType.CHAR:
                self.assertTrue(array.dtype.kind in {'U', 'S'})


    def test_get(self):

        pref = [0.57886104E+04, 0.57946934E+04, 0.58056177E+04]
        refI = [9, 9, 9]
        refJ = [4, 4, 4]
        refK = [1, 2, 3]

        rft1 = ERft(test_path("data/SPE1CASE1.RFT"))

        pres1a = rft1["PRESSURE", "A-1H", 2015, 9, 1 ]
        pres1b = rft1["PRESSURE", 2 ]

        self.assertEqual(len(pres1a), len(pref))
        self.assertEqual(len(pres1b), len(pref))

        for ref, v1, v2 in zip(pref, pres1a, pres1b):
            self.assertAlmostEqual(ref, v1, 4)
            self.assertAlmostEqual(ref, v2, 4)

        conI_1a = rft1["CONIPOS", "B-2H", 2016, 5, 31]
        conJ_1a = rft1["CONJPOS", "B-2H", 2016, 5, 31]
        conK_1a = rft1["CONKPOS", "B-2H", 2016, 5, 31]

        conI_1b = rft1["CONIPOS", 3]
        conJ_1b = rft1["CONJPOS", 3]
        conK_1b = rft1["CONKPOS", 3]

        self.assertEqual(len(refI), len(conI_1a))
        self.assertEqual(len(refI), len(conI_1b))

        self.assertEqual(len(refJ), len(conJ_1a))
        self.assertEqual(len(refJ), len(conJ_1b))

        self.assertEqual(len(refK), len(conK_1a))
        self.assertEqual(len(refK), len(conK_1b))

        for ref, v1, v2 in zip(refI, conI_1a, conI_1b):
            self.assertEqual(ref, v1)
            self.assertEqual(ref, v2)

        for ref, v1, v2 in zip(refJ, conJ_1a, conJ_1b):
            self.assertEqual(ref, v1)
            self.assertEqual(ref, v2)

        for ref, v1, v2 in zip(refK, conK_1a, conK_1b):
            self.assertEqual(ref, v1)
            self.assertEqual(ref, v2)


if __name__ == "__main__":

    unittest.main()


import unittest
import sys
import numpy as np

from opm.io.ecl import EclFile, eclArrType
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path



def array_index(ecl_file, target_kw):
    index_list = []
    for index,kw in enumerate(ecl_file.arrays):
        if kw[0] == target_kw:
            index_list.append(index)

    return index_list


class TestEclFile(unittest.TestCase):


    def test_arrays(self):

        refList=["INTEHEAD","LOGIHEAD","DOUBHEAD","PORV","DEPTH","DX","DY","DZ","PORO",
                 "PERMX","PERMY", "PERMZ","NTG","TRANX","TRANY","TRANZ","TABDIMS","TAB",
                 "ACTNUM","EQLNUM","FIPNUM","PVTNUM","SATNUM","TRANNNC"]

        self.assertRaises(ValueError, EclFile, "/file/that/does_not_exists")

        file2uf = EclFile(test_path("data/SPE9.INIT"), preload=False)
        self.assertEqual(len(file2uf), 24)

        arr_string_list = [ x[0] for x in file2uf.arrays ]
        self.assertEqual(arr_string_list, refList)

        file2f = EclFile(test_path("data/SPE9.FINIT"))
        self.assertEqual(len(file2f), 24)
        self.assertTrue(isinstance(file2uf.arrays, list))
        self.assertEqual(len(file2uf.arrays), len(refList))

        for str1, str2 in zip(file2uf.arrays, refList):
            self.assertEqual(str1[0], str2)

        self.assertEqual( file2uf.arrays[3] , ("PORV", eclArrType.REAL, 9000) )
        self.assertEqual( file2uf.arrays[16] , ("TABDIMS", eclArrType.INTE,100) )
        self.assertEqual( file2uf.arrays[17] , ("TAB", eclArrType.DOUB, 885) )

        
    def test_get_function(self):

        file1 = EclFile(test_path("data/SPE9.INIT"), preload=True)

        first = file1[0]
        self.assertEqual(len(first), 95)

        #fourth array in file SPE9.INIT which is PORV
        test1 = file1[3]
        porv_index = array_index(file1, "PORV")[0]
        test2 = file1[porv_index]

        for val1, val2 in zip(test1, test2):
            self.assertEqual(val1, val2)

    def test_get_function_float(self):

        file1 = EclFile(test_path("data/SPE9.INIT"))

        dzList=[20.0, 15.0, 26.0, 15.0, 16.0, 14.0, 8.0, 8.0, 18.0, 12.0, 19.0, 18.0, 20.0, 50.0, 100.0]
        poroList = [0.087, 0.097, 0.111, 0.16, 0.13, 0.17, 0.17, 0.08, 0.14, 0.13, 0.12, 0.105, 0.12, 0.116, 0.157]
        ft3_to_bbl = 0.1781076

        refporv = []

        for poro, dz in zip(dzList, poroList):
            for i in range(0,600):
                refporv.append(300.0*300.0*dz*poro*ft3_to_bbl)

        self.assertTrue("PORV" in file1)
        porv_index = array_index(file1, "PORV")[0]
        porv_np = file1[porv_index]

        self.assertEqual(len(porv_np), 9000)

        self.assertTrue(isinstance(porv_np, np.ndarray))
        self.assertEqual(porv_np.dtype, "float32")

        porv_list = file1[porv_index]

        for val1, val2 in zip(porv_np, refporv):
            self.assertLess(abs(1.0 - val1/val2), 1e-6)


    def test_get_function_double(self):

        refTabData=[0.147E+02, 0.2E+21, 0.4E+03, 0.2E+21, 0.8E+03, 0.2E+21, 0.12E+04, 0.2E+21, 0.16E+04, 0.2E+21, 0.2E+04, 0.2E+21, 0.24E+04, 0.2E+21, 0.28E+04, 0.2E+21, 0.32E+04, 0.2E+21, 0.36E+04, 0.2E+21, 0.4E+04, 0.5E+04, 0.1E+01, 0.2E+21, 0.98814229249012E+00, 0.2E+21, 0.97513408093613E+00]

        file1 = EclFile(test_path("data/SPE9.INIT"))
        tab_index = array_index(file1, "TAB")[0]

        tab = file1[tab_index]

        self.assertTrue(isinstance(tab, np.ndarray))
        self.assertEqual(tab.dtype, "float64")

        for i in range(0, len(refTabData)):
            self.assertLess(abs(1.0 - refTabData[i]/tab[i]), 1e-12 )


    def test_get_function_integer(self):

        refTabdims = [ 885, 1, 1, 1, 1, 1, 1, 67, 11, 2, 1, 78, 1, 78, 78, 0, 0, 0, 83, 1, 686, 40, 1, 86, 40, 1,
                      286, 1, 80, 1 ]

        file1 = EclFile(test_path("data/SPE9.INIT"))
        tabdims_index = array_index(file1, "TABDIMS")[0]
        tabdims = file1[tabdims_index]

        self.assertTrue(isinstance(tabdims, np.ndarray))
        self.assertEqual(tabdims.dtype, "int32")

        for i in range(0, len(refTabdims)):
            self.assertEqual(refTabdims[i], tabdims[i])


    def test_get_function_logi(self):

        file1 = EclFile(test_path("data/9_EDITNNC.INIT"))

        self.assertTrue("LOGIHEAD" in file1)
        logih_index = array_index(file1, "LOGIHEAD")[0]
        logih = file1[logih_index]

        self.assertEqual(len(logih), 121)
        self.assertEqual(logih[0], True)
        self.assertEqual(logih[2], False)
        self.assertEqual(logih[8], False)

    def test_get_function_char(self):

        file1 = EclFile(test_path("data/9_EDITNNC.SMSPEC"))

        self.assertTrue("KEYWORDS" in file1)
        kw_index = array_index(file1, "KEYWORDS")[0]
        keyw = file1[kw_index]

        self.assertEqual(len(keyw), 312)
        self.assertEqual(keyw[0], "TIME")
        self.assertEqual(keyw[16], "FWCT")


    def test_get_occurence(self):

        file1 = EclFile(test_path("data/SPE9.UNRST"))

        self.assertTrue("PRESSURE" in file1)

        with self.assertRaises(RuntimeError):
            test = file1["PRESSURE", int(10)]

        #first occurence of pressure
        pres = file1["PRESSURE"]
        pres0 = file1["PRESSURE", 0]

        self.assertEqual(len(pres), len(pres0))

        for v1, v2 in zip(pres, pres0):
            self.assertEqual(v1, v2)

        #occurence number 2 of pressure
        pres2 = file1["PRESSURE", 1]

        self.assertTrue(isinstance(pres2, np.ndarray))
        self.assertEqual(pres2.dtype, "float32")

        self.assertEqual(len(pres2), 9000)

        seqn0 = file1["SEQNUM", 0]
        self.assertEqual(seqn0[0], 37)

        seqn1 = file1["SEQNUM", 1]
        self.assertEqual(seqn1[0], 74)

        self.assertEqual(file1.count("PRESSURE"), 2)
        self.assertEqual(file1.count("XXXX"), 0)


if __name__ == "__main__":

    unittest.main()


import unittest
import sys
import numpy as np
import io
import os

from opm.io.ecl import EclOutput, EclFile, ERst, eclArrType
try:
    from tests.utils import test_path
except ImportError:
    from utils import test_path


class TestEclOutput(unittest.TestCase):

    def test_write_formatted_float32(self):

        npArr1=np.array([1.1, 2.2e+28, 3.3, 4.4], dtype='float32')

        testFile = test_path("data/RESULT.FINIT")

        # default is binary format (unformatted), new file
        out1 = EclOutput(testFile, formatted=True, append=False)
        out1.write("ARR1", npArr1)

        file1 = EclFile(testFile)
        testArray = file1[0]

        self.assertEqual(len(testArray), len(npArr1))

        for v1,v2 in zip (testArray, npArr1):
            self.assertEqual(v1, v2)

        if os.path.isfile(testFile):
            os.remove(testFile)


    def test_write_binary_float32(self):

        npArr1=np.array([1.1, 2.2e+28, 3.3, 4.4], dtype='float32')

        testFile = test_path("data/RESULT.INIT")

        # default is binary format (unformatted) and append = false
        out1 = EclOutput(testFile)
        out1.write("ARR1", npArr1)

        file1 = EclFile(testFile)
        testArray = file1[0]

        self.assertEqual(len(testArray), len(npArr1))

        for v1,v2 in zip (testArray, npArr1):
            self.assertEqual(v1, v2)

        if os.path.isfile(testFile):
            os.remove(testFile)


    def test_write_binary_append(self):

        npArr1 = np.array([1.1, 2.2e+28, 3.3, 4.4], dtype='float32')
        npArr2 = np.array([11.11, 12.12, 13.13, 14.14], dtype='float64')
        npArr3 = np.array([1,2,3,4], dtype='int32')
        npArr4 = np.array(["PROD1","","INJ1"], dtype='str')
        npArr5 = np.array([True, True, False, True, False, False], dtype='bool')
        #npArr6=np.array([11,12,13,14], dtype='int64')

        testFile = test_path("data/RESULT.INIT")

        # default is binary format (unformatted) and append = false
        # will create a new file1

        out1 = EclOutput(testFile)
        out1.write("ARR0", npArr1)

        # append = False (default value), will create a new file,
        # hence, array ARR0 from out1 will be errased

        out2 = EclOutput(testFile)
        out2.write("ARR1", npArr1)

        # append =True, will add ARR2 to existing file
        out3 = EclOutput(testFile, append=True)
        out3.write("ARR2", npArr2)
        out3.write("ARR3", npArr3)
        out3.write("ARR4", npArr4)
        out3.write("ARR5", npArr5)

        file1 = EclFile(testFile)

        self.assertEqual(len(file1), 5)

        # check array ARR1
        testArray = file1[0]
        self.assertEqual(len(testArray), len(npArr1))
        for v1,v2 in zip (testArray, npArr1):
            self.assertEqual(v1, v2)

        # check array ARR2
        testArray = file1[1]
        self.assertEqual(len(testArray), len(npArr2))
        for v1,v2 in zip (testArray, npArr2):
            self.assertEqual(v1, v2)

        # check array ARR3
        testArray = file1[2]
        self.assertEqual(len(testArray), len(npArr3))
        for v1,v2 in zip (testArray, npArr3):
            self.assertEqual(v1, v2)

        # check array ARR4
        testArray = file1[3]
        self.assertEqual(len(testArray), len(npArr4))
        for v1,v2 in zip (testArray, npArr4):
            self.assertEqual(v1, v2)

        # check array ARR5
        testArray = file1[4]
        self.assertEqual(len(testArray), len(npArr5))
        for v1,v2 in zip (testArray, npArr5):
            self.assertEqual(v1, v2)

        if os.path.isfile(testFile):
            os.remove(testFile)


    def test_write_formatted_append(self):

        npArr1 = np.array([1.1, 2.2e+28, 3.3, 4.4], dtype='float32')
        npArr2 = np.array([11.11, 12.12, 13.13, 14.14], dtype='float64')
        npArr3 = np.array([1,2,3,4], dtype='int32')
        npArr4 = np.array(["PROD1","","INJ1"], dtype='str')
        npArr5 = np.array([True, True, False, True, False, False], dtype='bool')
        #npArr6=np.array([11,12,13,14], dtype='int64')

        testFile = test_path("data/RESULT.FINIT")

        # default is binary format (unformatted) and append = false
        # will create a new file1

        out1 = EclOutput(testFile, formatted = True)
        out1.write("ARR0", npArr1)

        # append = False (default value), will create a new file,
        # hence, array ARR0 from out1 will be errased
        out2 = EclOutput(testFile, formatted = True)
        out2.write("ARR1", npArr1)

        # append =True, will add ARR2 to existing file
        out3 = EclOutput(testFile, append=True, formatted = True)
        out3.write("ARR2", npArr2)
        out3.write("ARR3", npArr3)
        out3.write("ARR4", npArr4)
        out3.write("ARR5", npArr5)

        file1 = EclFile(testFile)

        self.assertEqual(len(file1), 5)

        # check array ARR1
        testArray = file1[0]
        self.assertEqual(len(testArray), len(npArr1))
        for v1,v2 in zip (testArray, npArr1):
            self.assertEqual(v1, v2)

        # check array ARR2
        testArray = file1[1]
        self.assertEqual(len(testArray), len(npArr2))
        for v1,v2 in zip (testArray, npArr2):
            self.assertEqual(v1, v2)

        # check array ARR3
        testArray = file1[2]
        self.assertEqual(len(testArray), len(npArr3))
        for v1,v2 in zip (testArray, npArr3):
            self.assertEqual(v1, v2)

        # check array ARR4
        testArray = file1[3]
        self.assertEqual(len(testArray), len(npArr4))
        for v1,v2 in zip (testArray, npArr4):
            self.assertEqual(v1, v2)

        # check array ARR5
        testArray = file1[4]
        self.assertEqual(len(testArray), len(npArr5))
        for v1,v2 in zip (testArray, npArr5):
            self.assertEqual(v1, v2)

        if os.path.isfile(testFile):
            os.remove(testFile)


    def test_rewrite_rstfile_(self):

        rstep = 74
        rst1 = ERst(test_path("data/SPE9.UNRST"))
        arrayList74=rst1.arrays(rstep)

        outFile = test_path("data/TMP.UNRST")

        out1 = EclOutput(outFile)

        for n, (name, arrType, arrSize) in enumerate(arrayList74):
            if arrType == eclArrType.MESS:
                out1.write_message(name)
            else:
              array = rst1[name, rstep]
              out1.write(name, array)

        rst2 = ERst(outFile)
        tmpArrayList74=rst2.arrays(rstep)

        self.assertEqual(len(tmpArrayList74), len(arrayList74))

        for n in range(0, len(tmpArrayList74)):
            name1, arrType1, arrSize1 = tmpArrayList74[n]
            name2, arrType2, arrSize2 = arrayList74[n]

            self.assertEqual(name1, name2)
            self.assertEqual(arrType1, arrType2)

            if arrType1 != eclArrType.MESS:
                arr1 =  rst1[name1, rstep]
                arr2 =  rst2[name2, rstep]

                self.assertEqual(len(arr1), len(arr2))

                for v1,v2 in zip(arr1, arr2):
                    self.assertEqual(v1, v2)

        if os.path.isfile(outFile):
            os.remove(outFile)


    def test_write_lists(self):

        intList = [1,2,3,4,5,6]
        boolList = [True, True, False, True]
        strList = ["A-1H", "A-2H", "A-3H"]

        testFile = "data/TMP.DAT"
        outFile = test_path(testFile)
        out1 = EclOutput(outFile)

        out1.write("ARR1", intList)
        out1.write("ARR2", boolList)
        out1.write("ARR3", strList)

        file1 = EclFile(outFile)

        arr1 = file1["ARR1"]
        arr2 = file1["ARR2"]
        arr3 = file1["ARR3"]

        self.assertEqual(len(arr1), len(intList))
        self.assertEqual(len(arr2), len(boolList))
        self.assertEqual(len(arr3), len(strList))

        for v1, v2 in zip(arr1, intList):
           self.assertEqual(v1, v2)

        for v1, v2 in zip(arr2, boolList):
           self.assertEqual(v1, v2)

        for v1, v2 in zip(arr3, strList):
           self.assertEqual(v1, v2)

        if os.path.isfile(testFile):
            os.remove(testFile)



if __name__ == "__main__":

    unittest.main()


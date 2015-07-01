from ert.enkf.export import DesignMatrixReader
from ert.test import ExtendedTestCase
from ert.test.test_area import TestAreaContext

def dumpDesignMatrix1(path):
    with open(path, "w") as dm:
        dm.write("Case	CORR_SEIS_HEIMDAL	VOL_FRAC_HEIMDAL	AZIM_IND_HEIMDAL	VARIO_PARAL_HEIMDAL	VARIO_NORM_HEIMDAL	VARIO_VERT_HEIMDAL	SEIS_COND_HEIMDAL\n")
        dm.write("0	0.8			0.08			125			1000			500			25			ON\n")
        dm.write("1	0.8			0.15			125			2000			1000			25			ON\n")
        dm.write("2	0.8			0.20			125			4000			2000			25			ON\n")


def dumpDesignMatrix2(path):
    with open(path, "w") as dm:
        dm.write("Case	CORR_SEIS_HEIMDAL	VOL_FRAC_HEIMDAL	AZIM_IND_HEIMDAL	VARIO_PARAL_HEIMDAL	VARIO_NORM_HEIMDAL	VARIO_VERT_HEIMDAL	SEIS_COND_HEIMDAL\n")
        dm.write("0	0.8			0.08			125			1000			500			25			ON\n")
        dm.write("1	0.8			0.15			125			2000			1000			25			ON\n")
        dm.write("2	0.8			0.20			125			4000			2000			25			ON\n")
        dm.write("4	0.8			0.30			125			16000			8000			25			ON\n")



class DesignMatrixTest(ExtendedTestCase):

    def test_read_design_matrix(self):
        with TestAreaContext("python/enkf/export/design_matrix"):
            dumpDesignMatrix1("DesignMatrix.txt")
            dm = DesignMatrixReader.loadDesignMatrix("DesignMatrix.txt")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][0], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][0], 0.08)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][0], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][0], 1000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][0], 500)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][0], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][0], "ON")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][1], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][1], 0.15)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][1], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][1], 2000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][1], 1000)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][1], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][1], "ON")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][2], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][2], 0.20)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][2], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][2], 4000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][2], 2000)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][2], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][2], "ON")


    def test_read_design_matrix_2(self):
        with TestAreaContext("python/enkf/export/design_matrix_2"):
            dumpDesignMatrix2("DesignMatrix2.txt")
            dm = DesignMatrixReader.loadDesignMatrix("DesignMatrix2.txt")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][0], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][0], 0.08)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][0], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][0], 1000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][0], 500)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][0], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][0], "ON")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][1], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][1], 0.15)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][1], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][1], 2000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][1], 1000)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][1], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][1], "ON")

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][2], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][2], 0.20)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][2], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][2], 4000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][2], 2000)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][2], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][2], "ON")


            with self.assertRaises(KeyError):
                self.assertEqual(dm["CORR_SEIS_HEIMDAL"][3], 0.8)

            self.assertEqual(dm["CORR_SEIS_HEIMDAL"][4], 0.8)
            self.assertEqual(dm["VOL_FRAC_HEIMDAL"][4], 0.30)
            self.assertEqual(dm["AZIM_IND_HEIMDAL"][4], 125)
            self.assertEqual(dm["VARIO_PARAL_HEIMDAL"][4], 16000)
            self.assertEqual(dm["VARIO_NORM_HEIMDAL"][4], 8000)
            self.assertEqual(dm["VARIO_VERT_HEIMDAL"][4], 25)
            self.assertEqual(dm["SEIS_COND_HEIMDAL"][4], "ON")

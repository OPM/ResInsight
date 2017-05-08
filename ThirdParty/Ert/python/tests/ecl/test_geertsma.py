import datetime
from ecl.ecl import EclGrid, EclKW, EclDataType, openFortIO, FortIO, EclFile, EclSubsidence

from ecl.test import ExtendedTestCase , TestAreaContext

import numpy as np


def create_init(grid, case):
    poro = EclKW("PORO", grid.getNumActive(), EclDataType.ECL_FLOAT)
    porv = poro.copy()
    porv.setName("PORV")
    for g in range(grid.getGlobalSize()):
        porv[g] *= grid.cell_volume(global_index=g)

    with openFortIO("%s.INIT" % case, mode=FortIO.WRITE_MODE) as f:
        poro.fwrite(f)
        porv.fwrite(f)


def create_restart(grid, case, p1, p2=None):
    with openFortIO("%s.UNRST" % case, mode=FortIO.WRITE_MODE) as f:
        seq_hdr = EclKW("SEQNUM", 1, EclDataType.ECL_FLOAT)
        seq_hdr[0] = 10
        p = EclKW("PRESSURE", grid.getNumActive(), EclDataType.ECL_FLOAT)
        for i in range(len(p1)):
            p[i] = p1[i]

        header = EclKW("INTEHEAD", 67, EclDataType.ECL_INT)
        header[64] = 1
        header[65] = 1
        header[66] = 2000

        seq_hdr.fwrite(f)
        header.fwrite(f)
        p.fwrite(f)

        if p2:
            seq_hdr[0] = 20
            header[66] = 2010
            for i in range(len(p2)):
                p[i] = p2[i]

            seq_hdr.fwrite(f)
            header.fwrite(f)
            p.fwrite(f)


class GeertsmaTest(ExtendedTestCase):

    @staticmethod
    def test_geertsma_kernel():
        grid = EclGrid.createRectangular(dims=(1, 1, 1), dV=(50, 50, 50))
        with TestAreaContext("Subsidence"):
            p1 = [1]
            create_restart(grid, "TEST", p1)
            create_init(grid, "TEST")

            init = EclFile("TEST.INIT")
            restart_file = EclFile("TEST.UNRST")

            restart_view1 = restart_file.restartView(sim_time=datetime.date(2000, 1, 1))

            subsidence = EclSubsidence(grid, init)
            subsidence.add_survey_PRESSURE("S1", restart_view1)

            youngs_modulus = 5E8
            poisson_ratio = 0.3
            seabed = 0
            above = 100
            topres = 2000
            receiver = (1000, 1000, 0)

            dz = subsidence.evalGeertsma("S1", None, receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz, 3.944214576168326e-09)

            receiver = (1000, 1000, topres - seabed - above)

            dz = subsidence.evalGeertsma("S1", None, receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz, 5.8160298201497136e-08)

    @staticmethod
    def test_geertsma_kernel_2_source_points_2_vintages():
        grid = EclGrid.createRectangular(dims=(2, 1, 1), dV=(100, 100, 100))

        with TestAreaContext("Subsidence"):
            p1 = [1, 10]
            p2 = [10, 20]
            create_restart(grid, "TEST", p1, p2)
            create_init(grid, "TEST")

            init = EclFile("TEST.INIT")
            restart_file = EclFile("TEST.UNRST")

            restart_view1 = restart_file.restartView(sim_time=datetime.date(2000, 1, 1))
            restart_view2 = restart_file.restartView(sim_time=datetime.date(2010, 1, 1))

            subsidence = EclSubsidence(grid, init)
            subsidence.add_survey_PRESSURE("S1", restart_view1)
            subsidence.add_survey_PRESSURE("S2", restart_view2)

            youngs_modulus = 5E8
            poisson_ratio = 0.3
            seabed = 0
            receiver = (1000, 1000, 0)

            dz1 = subsidence.evalGeertsma("S1", None, receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz1, 8.65322541521704e-07)

            dz2 = subsidence.evalGeertsma("S2", None, receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz2, 2.275556615015282e-06)

            np.testing.assert_almost_equal(dz1-dz2, -1.4102340734935779e-06)

            dz = subsidence.evalGeertsma("S1", "S2", receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz, dz1-dz2)

    @staticmethod
    def test_geertsma_kernel_seabed():
        grid = EclGrid.createRectangular(dims=(1, 1, 1), dV=(50, 50, 50))
        with TestAreaContext("Subsidence"):
            p1 = [1]
            create_restart(grid, "TEST", p1)
            create_init(grid, "TEST")

            init = EclFile("TEST.INIT")
            restart_file = EclFile("TEST.UNRST")

            restart_view1 = restart_file.restartView(sim_time=datetime.date(2000, 1, 1))

            subsidence = EclSubsidence(grid, init)
            subsidence.add_survey_PRESSURE("S1", restart_view1)

            youngs_modulus = 5E8
            poisson_ratio = 0.3
            seabed = 300
            above = 100
            topres = 2000
            receiver = (1000, 1000, topres - seabed - above)

            dz = subsidence.evalGeertsma("S1", None, receiver, youngs_modulus, poisson_ratio, seabed)
            np.testing.assert_almost_equal(dz, 5.819790154474284e-08)



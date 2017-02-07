import time
import os.path
import sys

from ert.enkf import EnkfVarType
from ert.enkf.enums import RealizationStateEnum
from ert.server import SimulationContext
from ert.test import ErtTestContext, ExtendedTestCase
from tests.ert.server import initializeCase


class SimulationContextTest(ExtendedTestCase):

    def setUp(self):
        self.config1 = self.createTestPath("local/snake_oil_no_data/snake_oil.ert")
        self.config2 = self.createTestPath("local/snake_oil_no_data/snake_oil_GEO_ID.ert")


    def test_simulation_context(self):
        with ErtTestContext("ert/server/rpc/simulation_context", self.config1) as test_context:
            ert = test_context.getErt()
            
            size = 4
            first_half = initializeCase(ert, "first_half", size)
            other_half = initializeCase(ert, "other_half", size)

            simulation_context = SimulationContext(ert, size)

            for iens in range(size):
                if iens % 2 == 0:
                    simulation_context.addSimulation(iens, first_half)
                else:
                    simulation_context.addSimulation(iens, other_half)
                self.assertFalse(simulation_context.isRealizationFinished(iens))

            with self.assertRaises(UserWarning):
                simulation_context.addSimulation(size, first_half)

            with self.assertRaises(UserWarning):
                simulation_context.addSimulation(0, first_half)

            while simulation_context.isRunning():
                time.sleep(1.0)

            self.assertEqual(simulation_context.getNumFailed(), 0)
            self.assertEqual(simulation_context.getNumRunning(), 0)
            self.assertEqual(simulation_context.getNumSuccess(), size)

            first_half_state_map = first_half.getStateMap()
            other_half_state_map = other_half.getStateMap()

            for iens in range(size):
                self.assertTrue(simulation_context.didRealizationSucceed(iens))
                self.assertFalse(simulation_context.didRealizationFail(iens))
                self.assertTrue(simulation_context.isRealizationFinished(iens))
                if iens % 2 == 0:
                    self.assertEqual(first_half_state_map[iens], RealizationStateEnum.STATE_HAS_DATA)
                    self.assertEqual(other_half_state_map[iens], RealizationStateEnum.STATE_INITIALIZED)
                else:
                    self.assertEqual(first_half_state_map[iens], RealizationStateEnum.STATE_INITIALIZED)
                    self.assertEqual(other_half_state_map[iens], RealizationStateEnum.STATE_HAS_DATA)

            pfx = 'SimulationContext('
            self.assertEqual(pfx, repr(simulation_context)[:len(pfx)])

    def test_runpath(self):
        with ErtTestContext("ert/server/rpc/simulation_context_runpath", self.config2) as test_context:
            ert = test_context.getErt()
            sys.stderr.write("cwd: %s \n" % os.getcwd())
            size = 10

            fs = ert.getEnkfFsManager().getCurrentFileSystem()
            parameters = ert.ensembleConfig().getKeylistFromVarType(EnkfVarType.PARAMETER)
            ert.getEnkfFsManager().initializeFromScratch(parameters, 0, size - 1)
            simulation_context = SimulationContext(ert, size)
            
            for iens in range(size):
                state = ert.getRealisation(iens)
                if iens % 2 == 0:
                    state.addSubstKeyword("GEO_ID", "EVEN")
                else:
                    state.addSubstKeyword("GEO_ID", "ODD")
                simulation_context.addSimulation(iens , fs)

            while simulation_context.isRunning():
                time.sleep(1.0)

            for iens in range(size):
                if iens % 2 == 0:
                    path = "simulations/EVEN/realisation-%d/iter-%d" % (iens , 0)
                    self.assertTrue( os.path.isdir(path) )
                else:
                    path = "simulations/ODD/realisation-%d/iter-%d" % (iens , 0)
                    self.assertTrue( os.path.isdir(path) )

            pfx = 'SimulationContext('
            self.assertEqual(pfx, repr(simulation_context)[:len(pfx)])

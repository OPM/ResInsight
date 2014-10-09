from ert.enkf.enums.realization_state_enum import RealizationStateEnum
from ert.test import ErtTestContext, ExtendedTestCase
from ert.util import BoolVector


class LoadResultsManuallyTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/with_data/config")

    def test_load_results_manually(self):
        with ErtTestContext("manual_load_test", self.config_file) as test_context:
            ert = test_context.getErt()
            load_into_case = "A1"
            load_from_case = "default"

            load_into =  ert.getEnkfFsManager().getFileSystem(load_into_case)
            load_from =  ert.getEnkfFsManager().getFileSystem(load_from_case)

            ert.getEnkfFsManager().switchFileSystem(load_from)
            realisations = BoolVector(default_value=True,initial_size=25)
            realisations[7] = False
            iteration = 0

            ert.loadFromForwardModel(realisations, iteration, load_into)

            load_into_case_state_map = load_into.getStateMap()

            load_into_states = [state for state in load_into_case_state_map]

            expected = [RealizationStateEnum.STATE_HAS_DATA] * 25
            expected[7] = RealizationStateEnum.STATE_UNDEFINED
            
            self.assertListEqual(load_into_states, expected)












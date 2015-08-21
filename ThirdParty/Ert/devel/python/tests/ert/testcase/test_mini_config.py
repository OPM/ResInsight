from ert.enkf.enums.realization_state_enum import RealizationStateEnum
from ert.test import ErtTestContext
from ert.test.extended_testcase import ExtendedTestCase


class MiniConfigTest(ExtendedTestCase):

    def test_failed_realizations(self):

        # mini_fail_config has the following realization success/failures:
        #
        # 0 OK
        # 1 GenData report step 1 missing
        # 2 GenData report step 2 missing, Forward Model Component Target File not found.
        # 3 GenData report step 3 missing, Forward Model Component Target File not found.
        # 4 GenData report step 1 missing
        # 5 GenData report step 2 missing, Forward Model Component Target File not found.
        # 6 GenData report step 3 missing
        # 7 Forward Model Target File not found.
        # 8 OK
        # 9 OK


        config = self.createTestPath("local/custom_kw/mini_fail_config")
        with ErtTestContext("python/enkf/data/custom_kw_simulated", config) as context:
            ert = context.getErt()

            fs = ert.getEnkfFsManager().getCurrentFileSystem()

            realizations_list = fs.realizationList(RealizationStateEnum.STATE_HAS_DATA)
            self.assertTrue(0 in realizations_list)
            self.assertTrue(8 in realizations_list)
            self.assertTrue(9 in realizations_list)

            realizations_list = fs.realizationList(RealizationStateEnum.STATE_LOAD_FAILURE)
            self.assertTrue(1 in realizations_list)
            self.assertTrue(2 in realizations_list)
            self.assertTrue(3 in realizations_list)
            self.assertTrue(4 in realizations_list)
            self.assertTrue(5 in realizations_list)
            self.assertTrue(6 in realizations_list)
            self.assertTrue(7 in realizations_list)








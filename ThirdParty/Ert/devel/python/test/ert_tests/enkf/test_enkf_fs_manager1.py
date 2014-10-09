import os
from ert.enkf import EnkfFs
from ert.enkf import EnKFMain
from ert.enkf import EnkfFsManager
from ert.test import ErtTestContext
from ert.test import ExtendedTestCase 


class EnKFFSManagerTest1(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/with_data/config")


    def test_create(self):
        # We are indirectly testing the create through the create
        # already in the enkf_main object. In principle we could
        # create a separate manager instance from the ground up, but
        # then the reference count will be weird.
        with ErtTestContext("enkf_fs_manager_create_test", self.config_file) as testContext:
            ert = testContext.getErt()
            fsm = ert.getEnkfFsManager()

            fs = fsm.getCurrentFileSystem()
            self.assertEqual(2, fs.refCount())
            self.assertEqual(1, fsm.getFileSystemCount())

            fs2 = fsm.getFileSystem("newFS")
            self.assertEqual(2, fsm.getFileSystemCount())
            self.assertEqual(1, fs2.refCount())


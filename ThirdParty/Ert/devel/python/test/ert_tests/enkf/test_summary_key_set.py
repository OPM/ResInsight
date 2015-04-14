import os
from ert.enkf import SummaryKeySet
from ert.enkf.enkf_fs import EnkfFs
from ert.enkf.enkf_main import EnKFMain
from ert.test import ExtendedTestCase
from ert.test.ert_test_context import ErtTestContext
from ert.test.test_area import TestAreaContext


class SummaryKeySetTest(ExtendedTestCase):

    def test_creation(self):

        keys = SummaryKeySet()

        self.assertEqual(len(keys), 0)

        self.assertTrue(keys.addSummaryKey("FOPT"))

        self.assertEqual(len(keys), 1)

        self.assertTrue("FOPT" in keys)

        self.assertItemsEqual(["FOPT"], keys.keys())

        self.assertTrue(keys.addSummaryKey("WWCT"))

        self.assertEqual(len(keys), 2)

        self.assertTrue("WWCT" in keys)

        self.assertItemsEqual(["WWCT", "FOPT"], keys.keys())



    def test_read_only_creation(self):
        with TestAreaContext("enkf/summary_key_set/read_only_write_test"):
            keys = SummaryKeySet()

            keys.addSummaryKey("FOPT")
            keys.addSummaryKey("WWCT")

            filename = "test.txt"
            keys.writeToFile(filename)

            keys_from_file = SummaryKeySet(filename, read_only=True)
            self.assertItemsEqual(keys.keys(), keys_from_file.keys())

            self.assertTrue(keys_from_file.isReadOnly())
            self.assertFalse(keys_from_file.addSummaryKey("WOPR"))


    def test_write_to_and_read_from_file(self):
        with TestAreaContext("enkf/summary_key_set/write_test"):
            keys = SummaryKeySet()

            keys.addSummaryKey("FOPT")
            keys.addSummaryKey("WWCT")

            filename = "test.txt"

            self.assertFalse(os.path.exists(filename))

            keys.writeToFile(filename)

            self.assertTrue(os.path.exists(filename))

            keys_from_file = SummaryKeySet(filename)
            self.assertItemsEqual(keys.keys(), keys_from_file.keys())


    def test_with_enkf_fs(self):
        config_file = self.createTestPath("Statoil/config/with_data/config")

        with TestAreaContext("enkf/summary_key_set/enkf_fs", store_area=True) as context:
            context.copy_parent_content(config_file)

            fs = EnkfFs("storage/default")
            summary_key_set = fs.getSummaryKeySet()
            summary_key_set.addSummaryKey("FOPT")
            summary_key_set.addSummaryKey("WWCT")
            summary_key_set.addSummaryKey("WOPR")
            fs.umount()

            ert = EnKFMain("config")
            fs = ert.getEnkfFsManager().getCurrentFileSystem()
            summary_key_set = fs.getSummaryKeySet()
            self.assertTrue("FOPT" in summary_key_set)
            self.assertTrue("WWCT" in summary_key_set)
            self.assertTrue("WOPR" in summary_key_set)

            ensemble_config = ert.ensembleConfig()

            self.assertTrue("FOPT" in ensemble_config)
            self.assertTrue("WWCT" in ensemble_config)
            self.assertTrue("WOPR" in ensemble_config)
            self.assertFalse("TCPU" in ensemble_config)



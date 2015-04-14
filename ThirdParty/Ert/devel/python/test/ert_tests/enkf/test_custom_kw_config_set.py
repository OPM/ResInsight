import os
from ert.enkf import CustomKWConfigSet
from ert.enkf.data import CustomKWConfig
from ert.enkf.enkf_fs import EnkfFs
from ert.enkf.enkf_main import EnKFMain
from ert.test import ExtendedTestCase
from ert.test.ert_test_context import ErtTestContext
from ert.test.test_area import TestAreaContext
from ert.util.stringlist import StringList


class CustomKWConfigSetTest(ExtendedTestCase):

    def createCustomKWConfig(self, name, data):
        with TestAreaContext("python/enkf/custom_kw_config_set_config") as test_area:
            self.createResultFile("result_file", data)

            config = CustomKWConfig(name, "")
            config.parseResultFile("result_file", StringList())

        return config

    def createResultFile(self, filename, data):
        with open(filename, "w") as output_file:
            for key in data:
                output_file.write("%s %s\n" % (key, data[key]))


    def test_creation(self):
        config_set = CustomKWConfigSet()

        config = self.createCustomKWConfig("TEST", {"VALUE_1": 0.5, "VALUE_2": 5, "VALUE_3": "string", "VALUE_4": "true"})
        self.assertItemsEqual(config.getKeys(), ["VALUE_1", "VALUE_2", "VALUE_3", "VALUE_4"])

        config_set.addConfig(config)
        keys = config_set.getStoredConfigKeys()
        self.assertItemsEqual(keys, ["TEST"])

        config_set.reset()
        self.assertTrue(len(config_set.getStoredConfigKeys()) == 0)


    def test_fwrite_and_fread(self):
        with TestAreaContext("python/enkf/custom_kw_config_set_fwrite") as test_area:
            trees_config = self.createCustomKWConfig("TREES", {"OAK": 0.1, "SPRUCE": 5, "FIR": "pines", "PALM": "coconut"})
            insects_config = self.createCustomKWConfig("INSECTS", {"MOSQUITO": "annoying", "FLY": 3.14, "BEETLE": 0.5})

            config_set = CustomKWConfigSet()
            config_set.addConfig(trees_config)
            config_set.addConfig(insects_config)

            self.assertItemsEqual(config_set.getStoredConfigKeys(), ["TREES", "INSECTS"])

            config_set.fwrite("config_set")

            self.assertTrue(os.path.exists("config_set"))

            config_set = CustomKWConfigSet("config_set")

            self.assertItemsEqual(config_set.getStoredConfigKeys(), ["TREES", "INSECTS"])

            trees_config_from_file = CustomKWConfig("TREES", None)
            config_set.updateConfig(trees_config_from_file)

            for key in ["OAK", "SPRUCE", "FIR", "PALM"]:
                self.assertEqual(trees_config_from_file.indexOfKey(key), trees_config.indexOfKey(key))
                self.assertTrue(trees_config_from_file.keyIsDouble(key) == trees_config.keyIsDouble(key))


            insects_config_from_file = CustomKWConfig("INSECTS", None)
            config_set.updateConfig(insects_config_from_file)

            for key in ["MOSQUITO", "FLY", "BEETLE"]:
                self.assertEqual(insects_config_from_file.indexOfKey(key), insects_config.indexOfKey(key))
                self.assertTrue(insects_config_from_file.keyIsDouble(key) == insects_config.keyIsDouble(key))






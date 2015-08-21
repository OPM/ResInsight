import os
from ert.enkf.data import CustomKW, CustomKWConfig
from ert.enkf.enkf_simulation_runner import EnkfSimulationRunner
from ert.enkf.export import custom_kw_collector
from ert.enkf.export.custom_kw_collector import CustomKWCollector
from ert.test.ert_test_context import ErtTestContext
from ert.test.extended_testcase import ExtendedTestCase
from ert.test.test_area import TestAreaContext
from ert.util import StringList


class CustomKWTest(ExtendedTestCase):

    def createResultFile(self, filename, data):
        with open(filename, "w") as output_file:
            for key in data:
                output_file.write("%s %s\n" % (key, data[key]))

    def test_custom_kw_creation(self):
        data = {"VALUE_1": 2345.234,
                "VALUE_2": 0.001234,
                "VALUE_3": "string_1",
                "VALUE_4": "string_2"}

        with TestAreaContext("python/enkf/data/custom_kw_creation") as test_area:

            self.createResultFile("result_file", data)

            custom_kw_config = CustomKWConfig("CUSTOM_KW", "result_file")

            self.assertEqual(len(custom_kw_config), 0)

            custom_kw = CustomKW(custom_kw_config)

            custom_kw.fload("result_file")
            self.assertEqual(len(custom_kw_config), 4)

            for key in data:
                index = custom_kw_config.indexOfKey(key)
                self.assertEqual(data[key], custom_kw[key])

            with self.assertRaises(KeyError):
                value = custom_kw["VALUE_5"]



    def test_custom_kw_config_data_is_null(self):
            data_1 = {"VALUE_1": 123453.3,
                      "VALUE_2": 0.234234}

            data_2 = {"VALUE_1": 965689,
                      "VALUE_3": 1.1222}

            with TestAreaContext("python/enkf/data/custom_kw_null_element") as test_area:

                self.createResultFile("result_file_1", data_1)
                self.createResultFile("result_file_2", data_2)

                custom_kw_config = CustomKWConfig("CUSTOM_KW", "result_file")

                custom_kw_1 = CustomKW(custom_kw_config)
                custom_kw_1.fload("result_file_1")

                custom_kw_2 = CustomKW(custom_kw_config)
                custom_kw_2.fload("result_file_2")

                index_1 = custom_kw_config.indexOfKey("VALUE_1")
                index_2 = custom_kw_config.indexOfKey("VALUE_2")

                self.assertEqual(custom_kw_1["VALUE_1"], data_1["VALUE_1"])
                self.assertEqual(custom_kw_2["VALUE_1"], data_2["VALUE_1"])

                self.assertIsNone(custom_kw_2["VALUE_2"])
                self.assertFalse( "VALUE_3" in custom_kw_config )



    def test_simulated_custom_kw(self):
        config = self.createTestPath("local/custom_kw/mini_config")
        with ErtTestContext("python/enkf/data/custom_kw_simulated", config) as context:
            ert = context.getErt()

            ensemble_config = ert.ensembleConfig()
            self.assertTrue("AGGREGATED" in ensemble_config)

            config = ensemble_config.getNode("AGGREGATED").getCustomKeywordModelConfig()

            self.assertEqual(len(config.getKeys()), 0)

            simulation_runner = EnkfSimulationRunner(ert)
            simulation_runner.runEnsembleExperiment()

            config = ensemble_config.getNode("AGGREGATED").getCustomKeywordModelConfig()

            self.assertEqual(len(config.getKeys()), 4)
            self.assertItemsEqual(config.getKeys(), ["PERLIN_1", "PERLIN_2", "PERLIN_3", "STATE"])

from ert.enkf.export import GenDataObservationCollector
from ert.test import ErtTestContext, ExtendedTestCase


class GenDataObservationCollectorTest(ExtendedTestCase):

    def test_gen_data_collector(self):
        config = self.createTestPath("local/custom_kw/mini_config")
        with ErtTestContext("python/enkf/export/gen_data_observation_collector", config) as context:
            ert = context.getErt()

            obs_key = GenDataObservationCollector.getObservationKeyForDataKey(ert, "PERLIN", 1)
            self.assertEqual(obs_key, "GEN_PERLIN_1")

            obs_key = GenDataObservationCollector.getObservationKeyForDataKey(ert, "PERLIN", 2)
            self.assertEqual(obs_key, "GEN_PERLIN_2")

            obs_key = GenDataObservationCollector.getObservationKeyForDataKey(ert, "PERLIN", 3)
            self.assertEqual(obs_key, "GEN_PERLIN_3")

            obs_key = GenDataObservationCollector.getObservationKeyForDataKey(ert, "PERLIN", 4)
            self.assertIsNone(obs_key)

            obs_key = GenDataObservationCollector.getObservationKeyForDataKey(ert, "PERLINk", 1)
            self.assertIsNone(obs_key)

            data = GenDataObservationCollector.loadGenDataObservations(ert, "default", ["GEN_PERLIN_1", "GEN_PERLIN_3"])

            self.assertFloatEqual(data["GEN_PERLIN_1"][0], -0.616789)
            self.assertFloatEqual(data["STD_GEN_PERLIN_1"][0], 0.2)

            with self.assertRaises(KeyError):
                data["GEN_PERLIN_2"]
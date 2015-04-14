from ert.enkf import ErtImplType, GenKwConfig
from ert.test import ErtTestContext, ExtendedTestCase

class GenKwConfigTest(ExtendedTestCase):

    def setUp(self):
        self.config = self.createTestPath("Statoil/config/with_data/config")


    def test_gen_kw_config(self):

        with ErtTestContext("python/enkf/data/gen_kw_config", self.config) as context:

            ert = context.getErt()

            gen_kw_keys = ert.ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_KW)

            self.assertEqual(gen_kw_keys[0], "GRID_PARAMS")

            node = ert.ensembleConfig().getNode(gen_kw_keys[0])
            gen_kw_config = node.getModelConfig()
            self.assertIsInstance(gen_kw_config, GenKwConfig)

            self.assertEqual(gen_kw_config.getKey(), "GRID_PARAMS")
            self.assertEqual(len(gen_kw_config), 2)

            self.assertEqual(gen_kw_config[0], "MULTPV2")
            self.assertEqual(gen_kw_config[1], "MULTPV3")

            self.assertFalse(gen_kw_config.shouldUseLogScale(0))
            self.assertFalse(gen_kw_config.shouldUseLogScale(1))


            node = ert.ensembleConfig().getNode(gen_kw_keys[1])
            gen_kw_config = node.getModelConfig()
            self.assertIsInstance(gen_kw_config, GenKwConfig)

            self.assertEqual(gen_kw_config.getKey(), "MULTFLT")

            self.assertTrue(gen_kw_config.shouldUseLogScale(0))


            node = ert.ensembleConfig().getNode(gen_kw_keys[2])
            gen_kw_config = node.getModelConfig()
            self.assertIsInstance(gen_kw_config, GenKwConfig)

            self.assertEqual(gen_kw_config.getKey(), "FLUID_PARAMS")

            self.assertFalse(gen_kw_config.shouldUseLogScale(0))
            self.assertFalse(gen_kw_config.shouldUseLogScale(1))

            expected = ["SWCR", "SGCR"]

            for index, keyword in enumerate(gen_kw_config):
                self.assertEqual(keyword, expected[index])
from ert.cwrap import clib, CWrapper
from ert.enkf.data.enkf_node import EnkfNode
from ert.enkf.enums.enkf_state_type_enum import EnkfStateType
from ert.enkf.node_id import NodeId
from ert.test import ErtTestContext
from ert.test.extended_testcase import ExtendedTestCase
from ert.util import BoolVector

test_lib  = clib.ert_load("libenkf")
cwrapper =  CWrapper(test_lib)

get_active_mask = cwrapper.prototype("bool_vector_ref gen_data_config_get_active_mask( gen_data_config )")
update_active_mask = cwrapper.prototype("void gen_data_config_update_active( gen_data_config, int, bool_vector)")

class GenDataConfigTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/with_GEN_DATA/config")

    def load_active_masks(self, case1, case2 ):
        with ErtTestContext("gen_data_config_test", self.config_file) as test_context:
            ert = test_context.getErt()

            fs1 =  ert.getEnkfFsManager().getFileSystem(case1)
            config_node = ert.ensembleConfig().getNode("TIMESHIFT")
            data_node = EnkfNode(config_node)
            data_node.tryLoad(fs1, NodeId(60, 0, EnkfStateType.FORECAST))

            active_mask = get_active_mask( config_node.getDataModelConfig() )
            first_active_mask_length = len(active_mask)
            self.assertEqual(first_active_mask_length, 2560)

            fs2 =  ert.getEnkfFsManager().getFileSystem(case2)
            data_node = EnkfNode(config_node)
            data_node.tryLoad(fs2, NodeId(60, 0, EnkfStateType.FORECAST))

            active_mask = get_active_mask( config_node.getDataModelConfig() )
            second_active_mask_len = len(active_mask)
            self.assertEqual(second_active_mask_len, 2560)
            self.assertEqual(first_active_mask_length, second_active_mask_len)

            # Setting one element to False, load different case, check, reload, and check.
            self.assertTrue(BoolVector.cNamespace().iget(active_mask, 10))
            active_mask_modified = active_mask.copy()
            active_mask_modified[10] = False

            # Must switch filesystem, because the update mask (writes to storage)
            # functionality uses the current filesystem (current case)
            ert.getEnkfFsManager().switchFileSystem(fs2)
            update_active_mask(config_node.getDataModelConfig(),  60, active_mask_modified)
            active_mask = get_active_mask( config_node.getDataModelConfig() )
            self.assertFalse(active_mask[10])

            #Load first - check element is true
            data_node = EnkfNode(config_node)
            data_node.tryLoad(fs1, NodeId(60, 0, EnkfStateType.FORECAST))
            active_mask = get_active_mask( config_node.getDataModelConfig() )
            self.assertTrue(active_mask[10])

            # Reload second again, should now be false at 10, due to the update further up
            data_node = EnkfNode(config_node)
            data_node.tryLoad(fs2, NodeId(60, 0, EnkfStateType.FORECAST))
            active_mask = get_active_mask( config_node.getDataModelConfig() )
            self.assertFalse(active_mask[10])


    def test_loading_two_cases_with_and_without_active_file(self):
        self.load_active_masks("missing-active", "default")








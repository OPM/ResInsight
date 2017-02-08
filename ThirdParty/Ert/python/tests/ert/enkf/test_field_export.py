import os

from ert.util import IntVector

from ert.ecl import EclGrid

from ert.enkf.config import FieldTypeEnum, FieldConfig
from ert.enkf.data import EnkfNode
from ert.enkf.enums import EnkfFieldFileFormatEnum
from ert.enkf import NodeId

from ert.test import ExtendedTestCase
from ert.test import ErtTestContext


class FieldExportTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/obs_testing/config")

    def test_field_type_enum(self):
        with ErtTestContext("export_test", self.config_file) as test_context:
            ert = test_context.getErt()
            ens_config = ert.ensembleConfig()
            fc = ens_config["PERMX"].getFieldModelConfig()
            self.assertEqual(FieldTypeEnum.ECLIPSE_PARAMETER, fc.get_type())

    def test_field_basics(self):
        with ErtTestContext("export_test", self.config_file) as test_context:
            ert = test_context.getErt()
            ens_config = ert.ensembleConfig()
            fc = ens_config["PERMX"].getFieldModelConfig()
            pfx = 'FieldConfig(type'
            rep = repr(fc)
            self.assertEqual(pfx, rep[:len(pfx)])
            fc_xyz = fc.get_nx(),fc.get_ny(),fc.get_nz()
            ex_xyz = 40,64,14
            self.assertEqual(ex_xyz, fc_xyz)
            self.assertEqual(1,     fc.get_truncation_mode())
            self.assertEqual(0.001, fc.get_truncation_min())
            self.assertEqual(-1.0,  fc.get_truncation_max())
            self.assertEqual('LOG', fc.get_init_transform_name())
            self.assertEqual(None,  fc.get_output_transform_name())
            grid = fc.get_grid()
            self.assertEqual(ex_xyz, (grid.getNX(), grid.getNY(), grid.getNZ()))

    def test_field_export(self):
        with ErtTestContext("export_test", self.config_file) as test_context:
            ert = test_context.getErt()
            fs_manager = ert.getEnkfFsManager( )
            ens_config = ert.ensembleConfig()
            config_node = ens_config["PERMX"]
            data_node = EnkfNode( config_node )
            node_id = NodeId( 0 , 0 )
            fs = fs_manager.getCurrentFileSystem( )
            data_node.tryLoad( fs , node_id )

            data_node.export("export/with/path/PERMX.grdecl")
            self.assertTrue( os.path.isfile("export/with/path/PERMX.grdecl") )


    def test_field_export_many(self):
        with ErtTestContext("export_test", self.config_file) as test_context:
            ert = test_context.getErt()
            fs_manager = ert.getEnkfFsManager( )
            ens_config = ert.ensembleConfig()
            config_node = ens_config["PERMX"]
            iens_list = IntVector( )
            iens_list.append(0)
            iens_list.append(2)
            iens_list.append(4)

            fs = fs_manager.getCurrentFileSystem( )

            # Filename without embedded %d - TypeError
            with self.assertRaises(TypeError):
                EnkfNode.exportMany( config_node , "export/with/path/PERMX.grdecl" , fs , iens_list )

            EnkfNode.exportMany( config_node , "export/with/path/PERMX_%d.grdecl" , fs , iens_list )
            self.assertTrue( os.path.isfile("export/with/path/PERMX_0.grdecl") )
            self.assertTrue( os.path.isfile("export/with/path/PERMX_2.grdecl") )
            self.assertTrue( os.path.isfile("export/with/path/PERMX_4.grdecl") )

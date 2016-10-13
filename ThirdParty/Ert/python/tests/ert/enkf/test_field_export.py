import os

from ert.util import IntVector

from ert.ecl import EclGrid

from ert.enkf.config import FieldConfig
from ert.enkf.data import EnkfNode
from ert.enkf.enums import EnkfFieldFileFormatEnum
from ert.enkf import NodeId

from ert.test import ExtendedTestCase
from ert.test import ErtTestContext


class FieldExportTest(ExtendedTestCase):
    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/obs_testing/config")

        
    def test_export_format(self):
        self.assertEqual( FieldConfig.exportFormat("file.grdecl") , EnkfFieldFileFormatEnum.ECL_GRDECL_FILE ) 
        self.assertEqual( FieldConfig.exportFormat("file.roFF") , EnkfFieldFileFormatEnum.RMS_ROFF_FILE ) 

        with self.assertRaises(ValueError):
            FieldConfig.exportFormat("file.xyz")

        with self.assertRaises(ValueError):
            FieldConfig.exportFormat("file.xyz")

            
            
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


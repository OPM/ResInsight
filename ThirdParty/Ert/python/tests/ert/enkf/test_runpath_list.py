from os import path, symlink, remove

import ert
from cwrap import CWrapper
from ert.test import ExtendedTestCase, TestAreaContext,ErtTestContext
from ert.enkf import RunpathList, RunpathNode
from ert.util import BoolVector

test_lib  = ert.load("libenkf") # create a local namespace
cwrapper =  CWrapper(test_lib)

runpath_list_alloc = cwrapper.prototype("runpath_list_obj runpath_list_alloc(char*)")

class RunpathListTest(ExtendedTestCase):

    def test_runpath_list(self):
        runpath_list = runpath_list_alloc("")
        """ @type runpath_list: RunpathList """

        self.assertEqual(len(runpath_list), 0)

        test_runpath_nodes = [RunpathNode(0, 0, "runpath0", "basename0"), RunpathNode(1, 0, "runpath1", "basename0")]

        runpath_node = test_runpath_nodes[0]
        runpath_list.add(runpath_node.realization, runpath_node.iteration, runpath_node.runpath, runpath_node.basename)

        self.assertEqual(len(runpath_list), 1)
        self.assertEqual(runpath_list[0], test_runpath_nodes[0])

        runpath_node = test_runpath_nodes[1]
        runpath_list.add(runpath_node.realization, runpath_node.iteration, runpath_node.runpath, runpath_node.basename)

        self.assertEqual(len(runpath_list), 2)
        self.assertEqual(runpath_list[1], test_runpath_nodes[1])

        for index, runpath_node in enumerate(runpath_list):
            self.assertEqual(runpath_node, test_runpath_nodes[index])


        runpath_list.clear()

        self.assertEqual(len(runpath_list), 0)


        
    def test_sorted_export(self):
        with TestAreaContext("runpath_list"):
            runpath_list = RunpathList("EXPORT.txt")
            runpath_list.add( 3 , 1 , "path" , "base" )
            runpath_list.add( 1 , 1 , "path" , "base" )
            runpath_list.add( 2 , 1 , "path" , "base" )
            runpath_list.add( 0 , 0 , "path" , "base" )

            runpath_list.add( 3 , 0 , "path" , "base" )
            runpath_list.add( 1 , 0 , "path" , "base" )
            runpath_list.add( 2 , 0 , "path" , "base" )
            runpath_list.add( 0 , 1 , "path" , "base" )

            runpath_list.export( )

            path_list = []
            with open("EXPORT.txt") as f:
                for line in f.readlines():
                    tmp = line.split()
                    iens = int(tmp[0])
                    iteration = int(tmp[3])

                    path_list.append( (iens , iteration) )
                
            for iens in range(4):
                t0 = path_list[iens]
                t4 = path_list[iens + 4]
                self.assertEqual( t0[0] , iens )
                self.assertEqual( t4[0] , iens )

                self.assertEqual( t0[1] , 0 )
                self.assertEqual( t4[1] , 1 )


    def test_assert_export(self):
        with ErtTestContext("create_runpath1" , self.createTestPath("local/snake_oil_no_data/snake_oil.ert")) as tc:
            ert = tc.getErt( )
            runpath_list = ert.getRunpathList( )
            self.assertFalse( path.isfile( runpath_list.getExportFile( ) ))

            ens_size = ert.getEnsembleSize( )
            runner = ert.getEnkfSimulationRunner( )
            mask = BoolVector( initial_size = ens_size , default_value = True )
            runner.createRunPath( mask , 0 )

            self.assertTrue( path.isfile( runpath_list.getExportFile( ) ))
            self.assertEqual( "test_runpath_list.txt" , path.basename( runpath_list.getExportFile( ) ))
            

            
    def test_assert_symlink_deleted(self):
        with ErtTestContext("create_runpath2" , self.createTestPath("local/snake_oil_field/snake_oil.ert")) as tc:
            ert = tc.getErt( )
            runpath_list = ert.getRunpathList( )

            ens_size = ert.getEnsembleSize( )
            runner = ert.getEnkfSimulationRunner( )
            mask = BoolVector( initial_size = ens_size , default_value = True )

            # create directory structure
            runner.createRunPath( mask , 0 )

            # replace field file with symlink
            linkpath = '%s/permx.grdcel' % str(runpath_list[0].runpath)
            targetpath = '%s/permx.grdcel.target' % str(runpath_list[0].runpath)
            open(targetpath, 'a').close()
            remove(linkpath)
            symlink(targetpath, linkpath)

            # recreate directory structure
            runner.createRunPath( mask , 0 )

            # ensure field symlink is replaced by file
            self.assertFalse( path.islink(linkpath) )


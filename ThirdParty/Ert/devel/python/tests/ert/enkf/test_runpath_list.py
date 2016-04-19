from ert.cwrap import clib, CWrapper
from ert.test import ExtendedTestCase, TestAreaContext
from ert.enkf import RunpathList, RunpathNode


test_lib  = clib.ert_load("libenkf") # create a local namespace
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


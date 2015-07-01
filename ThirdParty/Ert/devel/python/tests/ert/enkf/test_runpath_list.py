from ert.cwrap import clib, CWrapper
from ert.test import ExtendedTestCase
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
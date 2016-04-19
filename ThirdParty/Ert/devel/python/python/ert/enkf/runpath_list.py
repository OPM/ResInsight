from collections import namedtuple
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB

RunpathNode = namedtuple("RunpathNode", ["realization", "iteration", "runpath", "basename"])

class RunpathList(BaseCClass):

    def __init__(self, export_file):
        c_ptr = RunpathList.cNamespace().alloc( export_file )
        super(RunpathList , self).__init__(c_ptr)

    def __len__(self):
        return RunpathList.cNamespace().size(self)

    def __getitem__(self, index):
        """ @rtype: RunpathNode """
        if not 0 <= index < len(self):
            raise IndexError("Index not in range: 0 <= %d < %d" % (index, len(self)))

        realization = RunpathList.cNamespace().iens(self, index)
        iteration = RunpathList.cNamespace().iteration(self, index)
        runpath = RunpathList.cNamespace().runpath(self, index)
        basename = RunpathList.cNamespace().basename(self, index)

        return RunpathNode(realization, iteration, runpath, basename)

    def __iter__(self):
        index = 0
        while index < len(self):
            yield self[index]
            index += 1


    def add(self, realization_number, iteration_number, runpath, basename):
        """
        @type realization_number: int
        @type iteration_number: int
        @type runpath: int
        @type basename: int
        """
        RunpathList.cNamespace().add(self, realization_number, iteration_number, runpath, basename)

    def clear(self):
        RunpathList.cNamespace().clear(self)


    def free(self):
        RunpathList.cNamespace().free(self)


    def export(self):
        RunpathList.cNamespace().export(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("runpath_list", RunpathList)

RunpathList.cNamespace().free = cwrapper.prototype("void runpath_list_free(runpath_list)")

RunpathList.cNamespace().add = cwrapper.prototype("void runpath_list_add(runpath_list, int, int, char*, char*)")
RunpathList.cNamespace().clear = cwrapper.prototype("void runpath_list_clear(runpath_list)")

RunpathList.cNamespace().size = cwrapper.prototype("int runpath_list_size(runpath_list)")
RunpathList.cNamespace().iens = cwrapper.prototype("int runpath_list_iget_iens(runpath_list, int)")
RunpathList.cNamespace().iteration = cwrapper.prototype("int runpath_list_iget_iter(runpath_list, int)")
RunpathList.cNamespace().runpath = cwrapper.prototype("char* runpath_list_iget_runpath(runpath_list, int)")
RunpathList.cNamespace().basename = cwrapper.prototype("char* runpath_list_iget_basename(runpath_list, int)")
RunpathList.cNamespace().export = cwrapper.prototype("void runpath_list_fprintf(runpath_list)")
RunpathList.cNamespace().alloc = cwrapper.prototype("c_void_p runpath_list_alloc(char*)")


from collections import namedtuple
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype

RunpathNode = namedtuple("RunpathNode", ["realization", "iteration", "runpath", "basename"])

class RunpathList(BaseCClass):
    TYPE_NAME = "runpath_list"
    _alloc     = EnkfPrototype("void* runpath_list_alloc(char*)" , bind = False)
    _free      = EnkfPrototype("void  runpath_list_free(runpath_list)")
    _add       = EnkfPrototype("void  runpath_list_add(runpath_list, int, int, char*, char*)")
    _clear     = EnkfPrototype("void  runpath_list_clear(runpath_list)")
    _size      = EnkfPrototype("int   runpath_list_size(runpath_list)")
    _iens      = EnkfPrototype("int   runpath_list_iget_iens(runpath_list, int)")
    _iteration = EnkfPrototype("int   runpath_list_iget_iter(runpath_list, int)")
    _runpath   = EnkfPrototype("char* runpath_list_iget_runpath(runpath_list, int)")
    _basename  = EnkfPrototype("char* runpath_list_iget_basename(runpath_list, int)")
    _export    = EnkfPrototype("void  runpath_list_fprintf(runpath_list)")

    _get_export_file = EnkfPrototype("char* runpath_list_get_export_file(runpath_list)")
    _set_export_file = EnkfPrototype("void runpath_list_set_export_file(runpath_list, char*)")

    def __init__(self, export_file):
        c_ptr = self._alloc( export_file )
        if c_ptr:
            super(RunpathList , self).__init__(c_ptr)
        else:
            raise ValueError('Could not construct RunpathList with export_file "%s".' % export_file)

    def __len__(self):
        return self._size( )

    def __getitem__(self, index):
        """ @rtype: RunpathNode """
        ls = len(self)
        if isinstance(index, int):
            idx = index
            if idx < 0:
                idx += ls
            if not 0 <= idx < ls:
                raise IndexError("Index not in range: 0 <= %d < %d" % (index, ls))
            realization = self._iens(idx)
            iteration   = self._iteration(idx)
            runpath     = self._runpath(idx)
            basename    = self._basename(idx)
            return RunpathNode(realization, iteration, runpath, basename)
        elif isinstance(index, slice):
            return [self[i] for i in range(*index.indices(ls))]
        raise TypeError('List indices must be integers, not %s.' % str(type(index)))


    def __iter__(self):
        index = 0
        while index < len(self):
            yield self[index]
            index += 1

    def getExportFile(self):
        return self._get_export_file( )


    def setExportFile(self , export_file):
        self._set_export_file( export_file )


    def add(self, realization_number, iteration_number, runpath, basename):
        """
        @type realization_number: int
        @type iteration_number: int
        @type runpath: int
        @type basename: int
        """
        self._add(realization_number, iteration_number, runpath, basename)

    def clear(self):
        self._clear( )


    def free(self):
        self._free( )

    def __repr__(self):
        return 'RunpathList(size = %d) %s' % (len(self), self._ad_str())

    def export(self):
        self._export( )

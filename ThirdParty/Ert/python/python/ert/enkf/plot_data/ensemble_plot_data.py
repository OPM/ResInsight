from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.config import EnkfConfigNode
from ert.enkf.enkf_fs import EnkfFs
from ert.util import BoolVector


class EnsemblePlotData(BaseCClass):
    TYPE_NAME = "ensemble_plot_data"

    _alloc = EnkfPrototype("void* enkf_plot_data_alloc(enkf_config_node)", bind = False)
    _load  = EnkfPrototype("void  enkf_plot_data_load(ensemble_plot_data, enkf_fs, char*, bool_vector)")
    _size  = EnkfPrototype("int   enkf_plot_data_get_size(ensemble_plot_data)")
    _get   = EnkfPrototype("ensemble_plot_data_vector_ref enkf_plot_data_iget(ensemble_plot_data, int)")
    _free  = EnkfPrototype("void  enkf_plot_data_free(ensemble_plot_data)")


    def __init__(self, ensemble_config_node, file_system=None, user_index=None, input_mask=None):
        assert isinstance(ensemble_config_node, EnkfConfigNode)

        c_pointer = self._alloc(ensemble_config_node)
        super(EnsemblePlotData, self).__init__(c_pointer)

        if not file_system is None:
            self.load(file_system, user_index, input_mask)


    def load(self, file_system, user_index=None, input_mask=None):
        assert isinstance(file_system, EnkfFs)
        if not input_mask is None:
            assert isinstance(input_mask, BoolVector)

        self._load(file_system, user_index, input_mask)

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def __getitem__(self, index):
        """ @rtype: EnsemblePlotDataVector """
        return self._get(index)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1


    def free(self):
        self._free()

    def __repr__(self):
        return 'EnsemblePlotData(size = %d) %s' % (len(self), self._ad_str())

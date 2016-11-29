from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.config import EnkfConfigNode
from ert.enkf.enkf_fs import EnkfFs
from ert.util import BoolVector


class EnsemblePlotData(BaseCClass):
    def __init__(self, ensemble_config_node, file_system=None, user_index=None, input_mask=None):
        assert isinstance(ensemble_config_node, EnkfConfigNode)

        c_pointer = EnsemblePlotData.cNamespace().alloc(ensemble_config_node)
        super(EnsemblePlotData, self).__init__(c_pointer)

        if not file_system is None:
            self.load(file_system, user_index, input_mask)


    def load(self, file_system, user_index=None, input_mask=None):
        assert isinstance(file_system, EnkfFs)
        if not input_mask is None:
            assert isinstance(input_mask, BoolVector)

        EnsemblePlotData.cNamespace().load(self, file_system, user_index, input_mask)

    def __len__(self):
        """ @rtype: int """
        return EnsemblePlotData.cNamespace().size(self)

    def __getitem__(self, index):
        """ @rtype: EnsemblePlotDataVector """
        return EnsemblePlotData.cNamespace().get(self, index)

    def __iter__(self):
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1


    def free(self):
        EnsemblePlotData.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ensemble_plot_data", EnsemblePlotData)
cwrapper.registerType("ensemble_plot_data_obj", EnsemblePlotData.createPythonObject)
cwrapper.registerType("ensemble_plot_data_ref", EnsemblePlotData.createCReference)

EnsemblePlotData.cNamespace().free = cwrapper.prototype("void enkf_plot_data_free(ensemble_plot_data)")
EnsemblePlotData.cNamespace().alloc = cwrapper.prototype("c_void_p enkf_plot_data_alloc(enkf_config_node)")
EnsemblePlotData.cNamespace().load = cwrapper.prototype("void enkf_plot_data_load(ensemble_plot_data, enkf_fs, char*, bool_vector)")
EnsemblePlotData.cNamespace().size = cwrapper.prototype("int enkf_plot_data_get_size(ensemble_plot_data)")
EnsemblePlotData.cNamespace().get = cwrapper.prototype("ensemble_plot_data_vector_ref enkf_plot_data_iget(ensemble_plot_data, int)")



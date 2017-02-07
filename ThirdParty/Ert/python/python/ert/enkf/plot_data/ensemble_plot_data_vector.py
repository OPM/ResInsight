from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import CTime



class EnsemblePlotDataVector(BaseCClass):
    TYPE_NAME = "ensemble_plot_data_vector"

    _size      = EnkfPrototype("int    enkf_plot_tvector_size(ensemble_plot_data_vector)")
    _get_value = EnkfPrototype("double enkf_plot_tvector_iget_value(ensemble_plot_data_vector, int)")
    _get_time  = EnkfPrototype("time_t enkf_plot_tvector_iget_time(ensemble_plot_data_vector, int)")
    _is_active = EnkfPrototype("bool   enkf_plot_tvector_iget_active(ensemble_plot_data_vector, int)")

    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def getValue(self, index):
        """ @rtype: float """
        return self._get_value(index)

    def getTime(self, index):
        """ @rtype: CTime """
        return self._get_time(index)

    def isActive(self, index):
        """ @rtype: bool """
        return self._is_active(index)

    def __repr__(self):
        return 'EnsemblePlotDataVector(size = %d) %s' % (len(self), self._ad_str())

from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import CTime



class EnsemblePlotDataVector(BaseCClass):
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")

    def __len__(self):
        """ @rtype: int """
        return EnsemblePlotDataVector.cNamespace().size(self)

    def getValue(self, index):
        """ @rtype: float """
        return EnsemblePlotDataVector.cNamespace().get_value(self, index)

    def getTime(self, index):
        """ @rtype: CTime """
        return EnsemblePlotDataVector.cNamespace().get_time(self, index)

    def isActive(self, index):
        """ @rtype: bool """
        return EnsemblePlotDataVector.cNamespace().is_active(self, index)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ensemble_plot_data_vector", EnsemblePlotDataVector)
cwrapper.registerType("ensemble_plot_data_vector_obj", EnsemblePlotDataVector.createPythonObject)
cwrapper.registerType("ensemble_plot_data_vector_ref", EnsemblePlotDataVector.createCReference)

EnsemblePlotDataVector.cNamespace().size = cwrapper.prototype("int enkf_plot_tvector_size(ensemble_plot_data_vector)")
EnsemblePlotDataVector.cNamespace().get_value = cwrapper.prototype("double enkf_plot_tvector_iget_value(ensemble_plot_data_vector, int)")
EnsemblePlotDataVector.cNamespace().get_time = cwrapper.prototype("time_t enkf_plot_tvector_iget_time(ensemble_plot_data_vector, int)")
EnsemblePlotDataVector.cNamespace().is_active = cwrapper.prototype("bool enkf_plot_tvector_iget_active(ensemble_plot_data_vector, int)")

from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.obs_data import ObsData
from ert.util import Matrix, IntVector


class MeasData(BaseCClass):

    def __init__(self, active_list):
        assert isinstance(active_list, IntVector)
        c_pointer = MeasData.cNamespace().alloc(active_list)
        super(MeasData, self).__init__(c_pointer)


    def createS(self, size):
        """ @rtype: Matrix """
        assert isinstance(size, int)
        return MeasData.cNamespace().allocS(self, size)

    def deactivateZeroStdSamples(self, obs_data):
        assert isinstance(obs_data, ObsData)
        self.cNamespace().deactivate_outliers(obs_data, self)


    def free(self):
        MeasData.cNamespace().free(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("meas_data", MeasData)
cwrapper.registerType("meas_data_obj", MeasData.createPythonObject)
cwrapper.registerType("meas_data_ref", MeasData.createCReference)

MeasData.cNamespace().alloc    = cwrapper.prototype("c_void_p meas_data_alloc(int_vector)")
MeasData.cNamespace().free     = cwrapper.prototype("void meas_data_free(meas_data)")

MeasData.cNamespace().allocS   = cwrapper.prototype("matrix_obj meas_data_allocS(meas_data, int)")

MeasData.cNamespace().deactivate_outliers  = cwrapper.prototype("void enkf_analysis_deactivate_std_zero(obs_data, meas_data)")




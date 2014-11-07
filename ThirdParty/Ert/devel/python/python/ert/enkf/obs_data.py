from types import NoneType
from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import Matrix


class ObsData(BaseCClass):

    def __init__(self):
        c_pointer = ObsData.cNamespace().alloc()
        super(ObsData, self).__init__(c_pointer)

    def __len__(self):
        """ @rtype: int """
        return ObsData.cNamespace().active_size(self)

    def createDobs(self, size):
        """ @rtype: Matrix """
        assert isinstance(size, int)
        return ObsData.cNamespace().allocdObs(self, size)

    def scale(self, S, E=None, D=None, R=None, D_obs=None):
        assert isinstance(S, Matrix)
        assert isinstance(E, (Matrix, NoneType))
        assert isinstance(D, (Matrix, NoneType))
        assert isinstance(R, (Matrix, NoneType))
        assert isinstance(D_obs, (Matrix, NoneType))
        ObsData.cNamespace().scale(self, S, E, D, R, D_obs)

    def free(self):
        ObsData.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("obs_data", ObsData)
cwrapper.registerType("obs_data_obj", ObsData.createPythonObject)
cwrapper.registerType("obs_data_ref", ObsData.createCReference)

ObsData.cNamespace().alloc       = cwrapper.prototype("c_void_p obs_data_alloc()")
ObsData.cNamespace().free        = cwrapper.prototype("void obs_data_free(obs_data)")
ObsData.cNamespace().active_size = cwrapper.prototype("int obs_data_get_active_size(obs_data)")

ObsData.cNamespace().allocdObs   = cwrapper.prototype("matrix_obj obs_data_allocdObs(obs_data, int)")
ObsData.cNamespace().scale       = cwrapper.prototype("void obs_data_scale(obs_data, matrix, matrix, matrix, matrix, matrix)")




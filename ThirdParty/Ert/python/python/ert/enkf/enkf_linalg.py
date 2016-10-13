from cwrap import CWrapper, CNamespace
from ert.enkf import ENKF_LIB
from ert.util import Matrix , DoubleVector


class EnkfLinalg(object):
    __namespace = CNamespace("EnkfLinalg")

    @classmethod
    def calculatePrincipalComponents(cls, S0, D_obs, truncation, ncomp, PC, PC_obs, singular_values):
        assert isinstance(S0, Matrix)
        assert isinstance(D_obs, Matrix)
        assert isinstance(truncation, (float, int))
        assert isinstance(ncomp, int)
        assert isinstance(PC, Matrix)
        assert isinstance(PC_obs, Matrix)
        assert isinstance(singular_values , DoubleVector)

        EnkfLinalg.cNamespace().get_PC(S0, D_obs, truncation, ncomp, PC, PC_obs , singular_values)

    @classmethod
    def cNamespace(cls):
        return EnkfLinalg.__namespace



cwrapper = CWrapper(ENKF_LIB)

EnkfLinalg.cNamespace().get_PC = cwrapper.prototype("void enkf_linalg_get_PC(matrix, matrix, double, int, matrix, matrix, double_vector)")




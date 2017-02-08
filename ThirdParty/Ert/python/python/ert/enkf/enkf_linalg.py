from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import Matrix , DoubleVector

class EnkfLinalg(BaseCClass):
    TYPE_NAME = "EnkfLinalg"

    _get_PC = EnkfPrototype("void enkf_linalg_get_PC(matrix, matrix, double, int, matrix, matrix, double_vector)", bind = False)

    @classmethod
    def calculatePrincipalComponents(cls, S0, D_obs, truncation, ncomp, PC, PC_obs, singular_values):
        assert isinstance(S0, Matrix)
        assert isinstance(D_obs, Matrix)
        assert isinstance(truncation, (float, int))
        assert isinstance(ncomp, int)
        assert isinstance(PC, Matrix)
        assert isinstance(PC_obs, Matrix)
        assert isinstance(singular_values , DoubleVector)

        cls._get_PC(S0, D_obs, truncation, ncomp, PC, PC_obs , singular_values)

from types import NoneType
from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import Matrix


class ObsData(BaseCClass):
    TYPE_NAME = "obs_data"

    _alloc         = EnkfPrototype("void*  obs_data_alloc(double)", bind = False)
    _free          = EnkfPrototype("void   obs_data_free(obs_data)")
    _total_size    = EnkfPrototype("int    obs_data_get_total_size(obs_data)")
    _scale         = EnkfPrototype("void   obs_data_scale(obs_data, matrix, matrix, matrix, matrix, matrix)")
    _scale_matrix  = EnkfPrototype("void   obs_data_scale_matrix(obs_data, matrix)")
    _scale_Rmatrix = EnkfPrototype("void   obs_data_scale_Rmatrix(obs_data, matrix)")
    _iget_value    = EnkfPrototype("double obs_data_iget_value(obs_data, int)")
    _iget_std      = EnkfPrototype("double obs_data_iget_std(obs_data, int)")
    _add_block     = EnkfPrototype("obs_block_ref obs_data_add_block(obs_data , char* , int , matrix , bool)")
    _allocdObs     = EnkfPrototype("matrix_obj obs_data_allocdObs(obs_data)")
    _allocR        = EnkfPrototype("matrix_obj obs_data_allocR(obs_data)")
    _allocD        = EnkfPrototype("matrix_obj obs_data_allocD(obs_data , matrix , matrix)")
    _allocE        = EnkfPrototype("matrix_obj obs_data_allocE(obs_data , rng , int)")

    def __init__(self, global_std_scaling=1.0):
        c_pointer = self._alloc(global_std_scaling)
        super(ObsData, self).__init__(c_pointer)

    def __len__(self):
        """ @rtype: int """
        return self._total_size()

    def __getitem__(self , index):
        if index < 0:
            index += len(self)

        if 0 <= index < len(self):
            value = self._iget_value( index )
            std = self._iget_std( index )
            return (value,std)

        raise IndexError("Invalid index:%d valid range: [0,%d)" % (index , len(self)))


    def __str__(self):
        s = ""
        for pair in self:
            s += "(%g, %g)\n" % pair
        return s

    def __repr__(self):
        return 'ObsData(total_size = %d) at 0x%x' % (len(self), self._address())

    def addBlock(self , obs_key , obs_size):
        error_covar = None 
        error_covar_owner = False
        return self._add_block(obs_key , obs_size , error_covar , error_covar_owner)


    def createDObs(self):
        """ @rtype: Matrix """
        return self._allocdObs()

    def createR(self):
        """ @rtype: Matrix """
        return self._allocR()

    def createD(self , E , S):
        """ @rtype: Matrix """
        return self._allocD(E , S)

    def createE( self , rng , active_ens_size):
        """ @rtype: Matrix """
        return self._allocE(rng , active_ens_size)

    def scaleMatrix(self, m):
        self._scale_matrix( m )


    def scaleRMatrix(self, R):
        self._scale_Rmatrix( R )


    def scale(self, S, E=None, D=None, R=None, D_obs=None):
        assert isinstance(S, Matrix)
        assert isinstance(E, (Matrix, NoneType))
        assert isinstance(D, (Matrix, NoneType))
        assert isinstance(R, (Matrix, NoneType))
        assert isinstance(D_obs, (Matrix, NoneType))
        self._scale(S, E, D, R, D_obs)


    def free(self):
        self._free()

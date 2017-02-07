from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.obs_data import ObsData
from ert.util import Matrix, IntVector , BoolVector


class MeasBlock(BaseCClass):
    TYPE_NAME = "meas_block"

    _alloc               = EnkfPrototype("void*  meas_block_alloc( char* , bool_vector , int)", bind = False)
    _free                = EnkfPrototype("void   meas_block_free( meas_block )")
    _get_active_ens_size = EnkfPrototype("int    meas_block_get_active_ens_size( meas_block )")
    _get_total_ens_size  = EnkfPrototype("int    meas_block_get_total_ens_size( meas_block )")
    _get_total_obs_size  = EnkfPrototype("int    meas_block_get_total_obs_size( meas_block )")
    _iget_value          = EnkfPrototype("double meas_block_iget( meas_block , int , int)")
    _iset_value          = EnkfPrototype("void   meas_block_iset( meas_block , int , int , double)")
    _iget_mean           = EnkfPrototype("double meas_block_iget_ens_mean( meas_block , int )")
    _iget_std            = EnkfPrototype("double meas_block_iget_ens_std( meas_block , int )")
    _iens_active         = EnkfPrototype("bool   meas_block_iens_active( meas_block , int )")

    def __init__(self , obs_key , obs_size , ens_mask):
        assert(isinstance(ens_mask , BoolVector))
        c_ptr = self._alloc( obs_key , ens_mask , obs_size )
        if c_ptr:
            super(MeasBlock , self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct MeasBlock.')

    def __str__(self):
        s = ""
        for iobs in range(self.getObsSize()):
            s += "["
            for iens in range(self.getTotalEnsSize()):
                if self.iensActive(iens):
                    s += "%6.3g " % self[iobs,iens]
                else:
                    s += "   X   "

            s += "]\n"
        return s

    def getObsSize(self):
        return self._get_total_obs_size()


    def getActiveEnsSize(self):
        return self._get_active_ens_size()


    def getTotalEnsSize(self):
        return self._get_total_ens_size()


    def __assert_index(self , index):
        if isinstance(index , tuple):
            iobs,iens = index
            if not 0 <= iobs < self.getObsSize():
                raise IndexError("Invalid iobs value:%d  Valid range: [0,%d)" % (iobs , self.getObsSize()))

            if not 0 <= iens < self.getTotalEnsSize():
                raise IndexError("Invalid iens value:%d  Valid range: [0,%d)" % (iobs , self.getTotalEnsSize()))

            if not self.iensActive( iens ):
                raise ValueError("Ensemble member:%d is not active - can not be accessed in the MeasBlock()" % iens)

            return iobs,iens
        else:
            raise TypeError("The index argument must be 2-tuple")


    def __setitem__(self, index, value):
        iobs , iens = self.__assert_index(index)
        self._iset_value( iens , iobs , value )


    def __getitem__(self, index):
        iobs,iens = self.__assert_index(index)
        return self._iget_value( iens , iobs )

    def iensActive(self , iens):
        return self._iens_active( iens )


    def free(self):
        self._free()


    def igetMean(self , iobs):
        if 0 <= iobs < self.getObsSize():
            return self._iget_mean(iobs)
        else:
            raise IndexError("Invalid observation index:%d  valid range: [0,%d)" % (iobs , self.getObsSize()))

    def igetStd(self , iobs):
        if 0 <= iobs < self.getObsSize():
            return self._iget_std(iobs)
        else:
            raise IndexError("Invalid observation index:%d  valid range: [0,%d)" % (iobs , self.getObsSize()))

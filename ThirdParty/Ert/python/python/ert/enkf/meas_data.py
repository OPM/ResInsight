from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.enkf.obs_data import ObsData
from ert.util import Matrix, IntVector


class MeasData(BaseCClass):
    TYPE_NAME = "meas_data"

    _alloc               = EnkfPrototype("void* meas_data_alloc(bool_vector)", bind = False)
    _free                = EnkfPrototype("void  meas_data_free(meas_data)")
    _get_active_obs_size = EnkfPrototype("int   meas_data_get_active_obs_size(meas_data)")
    _get_active_ens_size = EnkfPrototype("int   meas_data_get_active_ens_size( meas_data )")
    _get_total_ens_size  = EnkfPrototype("int   meas_data_get_total_ens_size( meas_data )")
    _get_num_blocks      = EnkfPrototype("int   meas_data_get_num_blocks( meas_data )")
    _has_block           = EnkfPrototype("bool  meas_data_has_block( meas_data , char* )")
    _get_block           = EnkfPrototype("meas_block_ref meas_data_get_block( meas_data , char*)")
    _allocS              = EnkfPrototype("matrix_obj     meas_data_allocS(meas_data)")
    _add_block           = EnkfPrototype("meas_block_ref meas_data_add_block(meas_data, char* , int , int)")
    _iget_block          = EnkfPrototype("meas_block_ref meas_data_iget_block( meas_data , int)")

    _deactivate_outliers = EnkfPrototype("void  enkf_analysis_deactivate_std_zero(obs_data, meas_data)", bind = False)

    def __init__(self, ens_mask):
        c_ptr = self._alloc(ens_mask)
        if c_ptr:
            super(MeasData, self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct MeasData from given ensemble mask of type %s.' % str(type(ens_mask)))

    def __len__(self):
        return self._get_num_blocks()


    def __contains__(self , index):
        if isinstance(index , str):
            return self._has_block(index)
        else:
            raise TypeError('The in operator expects a string argument, got "%s".' % str(index))


    def __getitem__(self , index):
        if isinstance(index , str):
            if index in self:
                return self._get_block(index)
            else:
                raise KeyError('The obs block "%s" is not recognized' % index)
        elif isinstance(index,int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                return self._iget_block(index)
            else:
                raise IndexError("Index out of range, should have 0 <= %d < %d." % (index, len(self)))
        else:
            raise TypeError("The index variable must string or integer")


    def __repr__(self):
        fmt = 'MeasData(len = %d, total ens = %d, active obs = %d, active ens = %d) at 0x%x'
        return fmt % (len(self), self.getTotalEnsSize(),
                      self.getActiveObsSize(), self.getActiveEnsSize(), self._address())

    def __str__(self):
        return '\n'.join([str(block) for block in self])


    def createS(self):
        """ @rtype: Matrix """
        S = self._allocS()
        if S is None:
            raise ValueError("Failed to create S active size : [%d,%d]" % (self.getActiveEnsSize() , self.activeObsSize( )))
        return S


    def deactivateZeroStdSamples(self, obs_data):
        assert isinstance(obs_data, ObsData)
        self._deactivate_outliers(obs_data, self)


    def addBlock(self , obs_key , report_step , obs_size):
        return self._add_block( obs_key , report_step , obs_size )


    def activeObsSize(self):
        return self._get_active_obs_size()


    def getActiveEnsSize(self):
        return self._get_active_ens_size()


    def getTotalEnsSize(self):
        return self._get_total_ens_size()


    def free(self):
        self._free()

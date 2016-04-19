from ert.cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.obs_data import ObsData
from ert.util import Matrix, IntVector


class MeasData(BaseCClass):

    def __init__(self, ens_mask):
        c_pointer = MeasData.cNamespace().alloc(ens_mask)
        super(MeasData, self).__init__(c_pointer)

    def __len__(self):
        return MeasData.cNamespace().get_num_blocks( self )


    def __contains__(self , index):
        if isinstance(index , str):
            return MeasData.cNamespace().has_block( self , index)
        else:
            raise TypeError("The in operator expects a string argument")
            

    def __getitem__(self , index):
        if isinstance(index , str):
            if index in self:
                return MeasData.cNamespace().get_block( self , index)
            else:
                raise KeyError("The obs block:%s is not recognized" % index)
        elif isinstance(index,int):
            if index < 0:
                index += len(self)

            if 0 <= index < len(self):
                return MeasData.cNamespace().iget_block( self , index)
            else:
                raise IndexError("Index out of range")
        else:
            raise TypeError("The index variable must string or integer")


    def __str__(self):
        s = ""
        for block in self:
            s += "%s" % block
            s += "\n"
        return s


    def createS(self):
        """ @rtype: Matrix """
        S = MeasData.cNamespace().allocS(self)
        if S is None:
            raise ValueError("Failed to create S active size : [%d,%d]" % (self.getActiveEnsSize() , self.activeObsSize( )))
        return S
    

    
    def deactivateZeroStdSamples(self, obs_data):
        assert isinstance(obs_data, ObsData)
        self.cNamespace().deactivate_outliers(obs_data, self)


    def addBlock(self , obs_key , report_step , obs_size):
        return MeasData.cNamespace().add_block( self , obs_key , report_step , obs_size )

        
    def activeObsSize(self):
        return MeasData.cNamespace().get_active_obs_size( self )

    
    def getActiveEnsSize(self):
        return MeasData.cNamespace().get_active_ens_size(self)


    def getTotalEnsSize(self):
        return MeasData.cNamespace().get_total_ens_size(self)


    def free(self):
        MeasData.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("meas_data", MeasData)

MeasData.cNamespace().alloc    = cwrapper.prototype("c_void_p meas_data_alloc(bool_vector)")
MeasData.cNamespace().free     = cwrapper.prototype("void meas_data_free(meas_data)")
MeasData.cNamespace().get_active_obs_size = cwrapper.prototype("int meas_data_get_active_obs_size(meas_data)")
MeasData.cNamespace().get_active_ens_size = cwrapper.prototype("int meas_data_get_active_ens_size( meas_data )")
MeasData.cNamespace().get_total_ens_size = cwrapper.prototype("int meas_data_get_total_ens_size( meas_data )")
MeasData.cNamespace().allocS    = cwrapper.prototype("matrix_obj meas_data_allocS(meas_data)")
MeasData.cNamespace().add_block = cwrapper.prototype("meas_block_ref meas_data_add_block(meas_data, char* , int , int)")
MeasData.cNamespace().get_num_blocks = cwrapper.prototype("int meas_data_get_num_blocks( meas_data )")
MeasData.cNamespace().has_block = cwrapper.prototype("bool meas_data_has_block( meas_data , char* )")
MeasData.cNamespace().get_block = cwrapper.prototype("meas_block_ref meas_data_get_block( meas_data , char*)")
MeasData.cNamespace().iget_block = cwrapper.prototype("meas_block_ref meas_data_iget_block( meas_data , int)")

MeasData.cNamespace().deactivate_outliers  = cwrapper.prototype("void enkf_analysis_deactivate_std_zero(obs_data, meas_data)")



from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.enkf.obs_data import ObsData
from ert.util import Matrix, IntVector , BoolVector


class MeasBlock(BaseCClass):

    def __init__(self , obs_key , obs_size , ens_mask):
        assert(isinstance(ens_mask , BoolVector))
        c_pointer = MeasBlock.cNamespace().alloc( obs_key , ens_mask , obs_size )
        super(MeasBlock , self).__init__(c_pointer)

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
        return MeasBlock.cNamespace().get_total_obs_size(self)


    def getActiveEnsSize(self):
        return MeasBlock.cNamespace().get_active_ens_size(self)


    def getTotalEnsSize(self):
        return MeasBlock.cNamespace().get_total_ens_size(self)

        
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
        MeasBlock.cNamespace().iset_value( self , iens , iobs , value )


    def __getitem__(self, index):
        iobs,iens = self.__assert_index(index)
        return MeasBlock.cNamespace().iget_value( self , iens , iobs )
        
    def iensActive(self , iens):
        return MeasBlock.cNamespace().iens_active( self , iens )


    def free(self):
        MeasBlock.cNamespace().free(self)


    def igetMean(self , iobs):
        if 0 <= iobs < self.getObsSize():
            return MeasBlock.cNamespace().iget_mean(self , iobs)
        else:
            raise IndexError("Invalid observation index:%d  valid range: [0,%d)" % (iobs , self.getObsSize()))

    def igetStd(self , iobs):
        if 0 <= iobs < self.getObsSize():
            return MeasBlock.cNamespace().iget_std(self , iobs)
        else:
            raise IndexError("Invalid observation index:%d  valid range: [0,%d)" % (iobs , self.getObsSize()))



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("meas_block", MeasBlock)

MeasBlock.cNamespace().alloc = cwrapper.prototype("c_void_p meas_block_alloc( char* , bool_vector , int)")
MeasBlock.cNamespace().free = cwrapper.prototype("void meas_block_free( meas_block )")
MeasBlock.cNamespace().get_active_ens_size = cwrapper.prototype("int meas_block_get_active_ens_size( meas_block )")
MeasBlock.cNamespace().get_total_ens_size = cwrapper.prototype("int meas_block_get_total_ens_size( meas_block )")
MeasBlock.cNamespace().get_total_obs_size = cwrapper.prototype("int meas_block_get_total_obs_size( meas_block )")
MeasBlock.cNamespace().iget_value = cwrapper.prototype("double meas_block_iget( meas_block , int , int)")
MeasBlock.cNamespace().iset_value = cwrapper.prototype("void meas_block_iset( meas_block , int , int , double)")
MeasBlock.cNamespace().iget_mean = cwrapper.prototype("double meas_block_iget_ens_mean( meas_block , int )")
MeasBlock.cNamespace().iget_std = cwrapper.prototype("double meas_block_iget_ens_std( meas_block , int )")
MeasBlock.cNamespace().iens_active = cwrapper.prototype("bool meas_block_iens_active( meas_block , int )")


    



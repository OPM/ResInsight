from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import Matrix


class ObsBlock(BaseCClass):

    def __init__(self , obs_key , obs_size , global_std_scaling=1.0):
        error_covar = None 
        error_covar_owner = False
        c_pointer = ObsBlock.cNamespace().alloc(obs_key , obs_size , error_covar , error_covar_owner, global_std_scaling)
        super(ObsBlock, self).__init__(c_pointer)


    def totalSize(self):
        return ObsBlock.cNamespace().total_size(self)

    def activeSize(self):
        return ObsBlock.cNamespace().active_size(self)

    def __setitem__(self , index , value):
        if isinstance(index , int):
            if 0 <= index < self.totalSize():
                if len(value) == 2:
                    d = value[0]
                    std = value[1]
                    
                    ObsBlock.cNamespace().iset(self , index , d , std)
                else:
                    raise TypeError("The value argument must be a two element tuple: (value , std)")
            else:
                raise IndexError("Invalid index:%d - valid range: [0,%d)" % (index , self.totalSize()))
        else:
            raise TypeError("The index item must be integer")


    def __getitem__(self , index):
        if isinstance(index , int):
            if 0 <= index < self.totalSize():
                value = ObsBlock.cNamespace().iget_value(self , index)
                std = ObsBlock.cNamespace().iget_std(self , index)
                
                return (value,std)
            else:
                raise IndexError("Invalid index:%d - valid range: [0,%d)" % (index , self.totalSize()))
        else:
            raise TypeError("The index item must be integer")



    def free(self):
        ObsBlock.cNamespace().free(self)



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("obs_block", ObsBlock)

ObsBlock.cNamespace().alloc = cwrapper.prototype("c_void_p obs_block_alloc(char*, int, matrix, bool, double)")
ObsBlock.cNamespace().free  = cwrapper.prototype("void obs_block_free(obs_block)")
ObsBlock.cNamespace().total_size  = cwrapper.prototype("int obs_block_get_size( obs_block )")
ObsBlock.cNamespace().active_size  = cwrapper.prototype("int obs_block_get_active_size( obs_block )")
ObsBlock.cNamespace().iset       = cwrapper.prototype("void obs_block_iset( obs_block , int , double , double)")
ObsBlock.cNamespace().iget_value = cwrapper.prototype("double obs_block_iget_value( obs_block , int)")
ObsBlock.cNamespace().iget_std   = cwrapper.prototype("double obs_block_iget_std( obs_block , int)")



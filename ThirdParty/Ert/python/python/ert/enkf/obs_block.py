from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import Matrix


class ObsBlock(BaseCClass):
    TYPE_NAME = "obs_block"

    _alloc       = EnkfPrototype("void*  obs_block_alloc(char*, int, matrix, bool, double)", bind = False)
    _free        = EnkfPrototype("void   obs_block_free(obs_block)")
    _total_size  = EnkfPrototype("int    obs_block_get_size( obs_block )")
    _active_size = EnkfPrototype("int    obs_block_get_active_size( obs_block )")
    _iset        = EnkfPrototype("void   obs_block_iset( obs_block , int , double , double)")
    _iget_value  = EnkfPrototype("double obs_block_iget_value( obs_block , int)")
    _iget_std    = EnkfPrototype("double obs_block_iget_std( obs_block , int)")

    def __init__(self , obs_key , obs_size , global_std_scaling=1.0):
        error_covar = None
        error_covar_owner = False
        c_pointer = self._alloc(obs_key , obs_size , error_covar , error_covar_owner, global_std_scaling)
        super(ObsBlock, self).__init__(c_pointer)


    def totalSize(self):
        return self._total_size()

    def activeSize(self):
        return self.active()
    def active(self):
        return self._active_size()
    def __len__(self):
        """Returns the total size"""
        return self.totalSize()

    def __setitem__(self , index , value):
        if len(value) != 2:
            raise TypeError("The value argument must be a two element tuple: (value , std)")
        d, std = value

        if isinstance(index , int):
            if index < 0:
                index += len(self)
            if 0 <= index < len(self):
                self._iset(index, d, std)
            else:
                raise IndexError("Invalid index: %d. Valid range: [0,%d)" % (index , len(self)))
        else:
            raise TypeError("The index item must be integer, not %s." % str(type(index)))


    def __getitem__(self , index):
        if isinstance(index , int):
            if index < 0:
                index += len(self)
            if 0 <= index < len(self):
                value = self._iget_value(index)
                std   = self._iget_std(index)
                return (value,std)
            else:
                raise IndexError("Invalid index:%d - valid range: [0,%d)" % (index , len(self)))
        else:
            raise TypeError("The index item must be integer, not %s." % str(type(index)))

    def free(self):
        self._free()

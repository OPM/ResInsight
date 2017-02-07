from cwrap import BaseCClass
from ert.util import DoubleVector
from ert.enkf import EnkfPrototype


class GenData(BaseCClass):
    TYPE_NAME = "gen_data"
    _alloc       = EnkfPrototype("void*  gen_data_alloc()", bind = False)
    _free        = EnkfPrototype("void   gen_data_free(gen_data)")
    _size        = EnkfPrototype("int    gen_data_get_size(gen_data)")
    _iget        = EnkfPrototype("double gen_data_iget_double(gen_data , int)");
    _export      = EnkfPrototype("void   gen_data_export(gen_data , char*, gen_data_file_format_type, fortio)")
    _export_data = EnkfPrototype("void   gen_data_export_data(gen_data , double_vector)")

    def __init__(self):
        c_ptr = self._alloc()
        if c_ptr:
            super(GenData, self).__init__(c_ptr)
        else:
            raise ValueError('Unable to construct GenData object.')


    def __len__(self):
        """ @rtype: int """
        return self._size()

    
    def free(self):
        self._free( )

    def __repr__(self):
        return 'GenData(len = %d) %s' % (len(self), self._ad_str())

    def export(self, file_name, file_format_type, fortio):
        """
        @type: str
        @type: GenDataFileType
        @type: FortIO
        """
        self._export(file_name, file_format_type, fortio)

        
    def getData(self):
        data = DoubleVector()
        self._export_data( data )
        return data

    def __getitem__( self, idx ):
        """Returns an item, or a list if idx is a slice.
        Note: When idx is a slice it does not return a new GenData!
        """
        ls = len(self)
        if isinstance( idx, int ):
            if idx < 0:
                idx += ls
            if 0 <= idx < ls:
                return self._iget(idx)
            raise IndexError('List index out of range.')
        if isinstance( idx, slice ):
            vec = self.getData()
            return [vec[i] for i in range(*idx.indices(ls))]
        raise TypeError('List indices must be integers, not %s.' % str(type(idx)))

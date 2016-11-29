from cwrap import BaseCClass, CWrapper
from ert.util import DoubleVector
from ert.enkf import ENKF_LIB



class GenData(BaseCClass):

    def __init__(self):
        c_pointer = GenData.cNamespace().alloc()
        super(GenData, self).__init__(c_pointer)

    def __len__(self):
        """ @rtype: int """
        return GenData.cNamespace().size(self)

    def free(self):
        GenData.cNamespace().free(self)

    def export(self, file_name, file_format_type, fortio):
        """
        @type: str
        @type: GenDataFileType
        @type: FortIO
        """
        GenData.cNamespace().export(self, file_name, file_format_type, fortio)
        
    def getData(self):
        data = DoubleVector()
        GenData.cNamespace().export_data( self , data )
        return data



cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerObjectType("gen_data", GenData)

GenData.cNamespace().alloc       = cwrapper.prototype("c_void_p gen_data_alloc()")
GenData.cNamespace().free        = cwrapper.prototype("void gen_data_free(gen_data)")
GenData.cNamespace().size = cwrapper.prototype("int gen_data_get_size(gen_data)")

GenData.cNamespace().export      = cwrapper.prototype("void gen_data_export(gen_data , char*, gen_data_file_format_type, fortio)")
GenData.cNamespace().export_data = cwrapper.prototype("void gen_data_export_data(gen_data , double_vector)")



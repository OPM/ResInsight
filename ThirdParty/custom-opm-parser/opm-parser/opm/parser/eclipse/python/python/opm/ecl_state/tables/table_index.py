import os.path

from ert.cwrap import BaseCClass
from opm import OPMPrototype


class TableIndex(BaseCClass):
    TYPE_NAME   = "table_index"
    _free       = OPMPrototype("void   table_index_free( table_index )")
    

    def __init__(self , *args, **kwargs):
        raise NotImplementedError("Can not create instantiate tables directly")


    def free(self):
        self._free( )

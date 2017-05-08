from cwrap import BaseCClass
from ecl.util import UtilPrototype


class PathFormat(BaseCClass):
    TYPE_NAME = "path_fmt"
    _str  = UtilPrototype("char* path_fmt_get_fmt(path_fmt)")
    _free = UtilPrototype("void path_fmt_free(path_fmt)") 
    
    def __init__(self):
        raise NotImplementedError("Class can not be instantiated directly!")


    def __str__(self):
        return self._str( )


    def free(self):
        self._free( )

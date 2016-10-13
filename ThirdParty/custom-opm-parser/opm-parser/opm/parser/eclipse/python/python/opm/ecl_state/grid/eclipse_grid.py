from cwrap import BaseCClass

from opm import OPMPrototype
from opm.deck import Deck

class EclipseGrid(BaseCClass):
    TYPE_NAME = "eclipse_grid"
    _alloc = OPMPrototype("void* eclipse_grid_alloc( deck )" , bind = False)
    _free  = OPMPrototype("void  eclipse_grid_free( eclipse_grid )")

    
    def __init__(self , deck):
        c_ptr = self._alloc( deck )
        super(EclipseGrid , self).__init__( c_ptr )


    def free(self):
        self._free( )

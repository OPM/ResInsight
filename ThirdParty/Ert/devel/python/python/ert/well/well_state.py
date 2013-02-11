import libwell
from   ert.cwrap.cwrap       import *

class WellState:
    def __init__(self):
        self.c_ptr = None




# 2. Creating a wrapper object around the libwell library, 
cwrapper = CWrapper( libwell.lib )
cwrapper.registerType( "well_state" , WellState )


# 3. Installing the c-functions used to manipulate ecl_file instances.
#    These functions are used when implementing the WellState class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("well_state")
        

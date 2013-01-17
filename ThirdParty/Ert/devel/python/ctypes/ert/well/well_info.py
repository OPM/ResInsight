import libwell
from   ert.cwrap.cwrap       import *
import well_state


class WellInfo:
    
    def __init__(self):
        pass

    


# 2. Creating a wrapper object around the libecl library, 
cwrapper = CWrapper( libwell.lib )
cwrapper.registerType( "well_info" , WellInfo )


# 3. Installing the c-functions used to manipulate ecl_file instances.
#    These functions are used when implementing the EclFile class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("well_info")



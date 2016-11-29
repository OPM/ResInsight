#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  The file 'forward_load_context.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

from ert.enkf import EnkfPrototype
from cwrap import BaseCClass


# The Python wrapping of the forward_load_context is extremely
# minimal; when creating the Python implementation the only purpose
# was to get an existing test through.


class ForwardLoadContext(BaseCClass):
    TYPE_NAME = "forward_load_context"
    _alloc = EnkfPrototype("void* forward_load_context_alloc( run_arg , bool , ecl_config , char* , stringlist )" , bind = False)
    _select_step = EnkfPrototype("void forward_load_context_select_step( forward_load_context , int )")
    _get_step = EnkfPrototype("int forward_load_context_get_load_step( forward_load_context)")
    _free = EnkfPrototype("void forward_load_context_free( forward_load_context)")

    
    def __init__(self , run_arg = None , load_summary = False , ecl_config = None , ecl_base = None , messages = None , report_step = None):
        c_ptr = self._alloc(run_arg , load_summary , ecl_config , ecl_base , messages)
        super(ForwardLoadContext, self).__init__(c_ptr)
        if not report_step is None:
            self.selectStep( report_step )


    def getLoadStep( self ):
        return self._get_step( )


    def selectStep(self , report_step):
        self._select_step( report_step) 
        

    def free(self):
        self._free( )


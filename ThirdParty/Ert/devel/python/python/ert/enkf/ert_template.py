#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ert_template.py' is part of ERT - Ensemble based Reservoir Tool. 
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

import  ctypes
from    ert.cwrap.cwrap       import *
from    ert.cwrap.cclass      import CClass
from    ert.util.tvector      import * 
from    enkf_enum             import *
import  libenkf
class ErtTemplate(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    @property
    def get_template_file(self):
        return cfunc.get_template_file(self)

    @property
    def get_target_file(self):
        return cfunc.get_target_file(self)

    @property
    def get_args_as_string(self):
        return cfunc.get_args_as_string(self)   
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "ert_template" , ErtTemplate )

cfunc = CWrapperNameSpace("ert_template")
##################################################################
##################################################################
cfunc.free                   = cwrapper.prototype("void ert_template_free( ert_template )")
cfunc.get_template_file      = cwrapper.prototype("char* ert_template_get_template_file(ert_template)")
cfunc.get_target_file        = cwrapper.prototype("char* ert_template_get_target_file(ert_template)")
cfunc.get_args_as_string     = cwrapper.prototype("char* ert_template_get_args_as_string(ert_template)")

#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'gen_kw_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    ert.enkf.enkf_enum             import *
import  ert.enkf.libenkf
from ert.util.stringlist import StringList
class GenKwConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    @property
    def get_template_file(self):
        return cfunc.get_template_file(self)

    @property
    def get_parameter_file(self):
        return cfunc.get_parameter_file(self)

    @property
    def alloc_name_list(self):
        return StringList(c_ptr = cfunc.alloc_name_list(self), parent = self)

##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "gen_kw_config" , GenKwConfig )

cfunc = CWrapperNameSpace("gen_kw_config")
##################################################################
##################################################################
cfunc.free                   = cwrapper.prototype("void gen_kw_config_free( gen_kw_config )")
cfunc.get_template_file      = cwrapper.prototype("char* gen_kw_config_get_template_file(gen_kw_config)")
cfunc.get_parameter_file     = cwrapper.prototype("char* gen_kw_config_get_parameter_file(gen_kw_config)")
cfunc.alloc_name_list        = cwrapper.prototype("c_void_p gen_kw_config_alloc_name_list(gen_kw_config)")

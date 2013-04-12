#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'config_parser.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from    config_enums          import *

class ConfigParser(CClass):
    
    def __init__(self):
        c_ptr = cfunc.alloc()
        self.init_cobj( c_ptr , cfunc.free )


    def parse(self , filename , comment_string = "--" , include_kw = "INCLUDE" , define_kw = None , unrecognized = CONFIG_UNRECOGNIZED_WARN , validate = True):
        return cfunc.parse(self , filename , comment_string , include_kw , define_kw , unrecognized , validate)

    
    def add(self , kw , required = False):
        cfunc.add_schema_item( self , kw , required )


#################################################################

cwrapper = CWrapper( libconfig.lib )
cwrapper.registerType( "config" , ConfigParser )
cfunc = CWrapperNameSpace("config")
cfunc.alloc           = cwrapper.prototype("c_void_p config_alloc()")
cfunc.free            = cwrapper.prototype("void     config_free( config )")
cfunc.parse           = cwrapper.prototype("bool     config_parse( config , char* , char* , char* , char*, int , bool)")
cfunc.add_schema_item = cwrapper.prototype("bool     config_add_schema_item( config , char* , bool)")

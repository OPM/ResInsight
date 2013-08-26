#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'ecl_config.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import  ert.util.libutil
from    ert.ecl.ecl_sum import EclSum
from ert.util.stringlist import StringList
class EclConfig(CClass):
    
    def __init__(self , c_ptr , parent = None):
        if parent:
            self.init_cref( c_ptr , parent)
        else:
            self.init_cobj( c_ptr , cfunc.free )

    @property
    def get_eclbase(self):
        eclbase = cfunc.get_eclbase(self)
        return eclbase

    @property
    def get_data_file(self):
        datafile = cfunc.get_data_file(self)
        return datafile
    
    @property
    def get_gridfile(self):
        gridfile = cfunc.get_gridfile(self)
        return gridfile

    def set_gridfile(self, gridfile):
        cfunc.set_gridfile(self, gridfile)

    @property
    def get_schedule_file(self):
        schedule_file = cfunc.get_schedule_file(self)
        return schedule_file

    def set_schedule_file(self, schedule_file):
        schedule_file = cfunc.set_schedule_file(self, schedule_file)

    @property
    def get_init_section(self):
        init_section = cfunc.get_init_section(self)
        return init_section

    def set_init_section(self, init_section):
        cfunc.set_init_section(self, init_section)

    @property
    def get_refcase_name(self):
        refcase_name = cfunc.get_refcase_name(self)
        return refcase_name

    def load_refcase(self, refcase):
        cfunc.load_refcase(self, refcase)
        
    @property     
    def get_static_kw_list(self):
        return StringList(c_ptr = cfunc.get_static_kw_list( self ) , parent = self)

    @property     
    def get_refcase(self):
        refcase = EclSum(self.get_refcase_name, join_string = ":" , include_restart = True, c_ptr = cfunc.get_refcase( self ), parent = self)
        return refcase
    
    def clear_static_kw(self):
        cfunc.clear_static_kw(self)

    def add_static_kw(self,kw):
        cfunc.add_static_kw(self,kw)

    @property
    def get_grid(self):
        return cfunc.get_grid(self)

    @property
    def get_sched_file(self):
        return cfunc.get_sched_file(self)
##################################################################

cwrapper = CWrapper( libenkf.lib )
cwrapper.registerType( "ecl_config" , EclConfig )

cfunc = CWrapperNameSpace("ecl_config")


cfunc.free               = cwrapper.prototype("void ecl_config_free( ecl_config )")
cfunc.get_eclbase        = cwrapper.prototype("char* ecl_config_get_eclbase( ecl_config )")
cfunc.get_data_file      = cwrapper.prototype("char* ecl_config_get_data_file(ecl_config)")
cfunc.get_gridfile       = cwrapper.prototype("char* ecl_config_get_gridfile(ecl_config)")
cfunc.set_gridfile       = cwrapper.prototype("void ecl_config_set_grid(ecl_config, char*)")
cfunc.get_schedule_file  = cwrapper.prototype("char* ecl_config_get_schedule_file(ecl_config)")
cfunc.set_schedule_file  = cwrapper.prototype("void ecl_config_set_schedule_file(ecl_config, char*)")
cfunc.get_init_section   = cwrapper.prototype("char* ecl_config_get_init_section(ecl_config)")
cfunc.set_init_section   = cwrapper.prototype("void ecl_config_set_init_section(ecl_config, char*)")
cfunc.get_refcase_name   = cwrapper.prototype("char* ecl_config_get_refcase_name(ecl_config)")
cfunc.load_refcase       = cwrapper.prototype("void ecl_config_load_refcase(ecl_config, char*)")
cfunc.get_static_kw_list = cwrapper.prototype("c_void_p ecl_config_get_static_kw_list(ecl_config)")
cfunc.clear_static_kw    = cwrapper.prototype("void ecl_config_clear_static_kw(ecl_config)")
cfunc.add_static_kw      = cwrapper.prototype("void ecl_config_add_static_kw(ecl_config, char*)")
cfunc.get_grid           = cwrapper.prototype("c_void_p ecl_config_get_grid(ecl_config)")
cfunc.get_refcase        = cwrapper.prototype("c_void_p ecl_config_get_refcase(ecl_config)")
cfunc.get_sched_file      = cwrapper.prototype("c_void_p ecl_config_get_sched_file(ecl_config)")

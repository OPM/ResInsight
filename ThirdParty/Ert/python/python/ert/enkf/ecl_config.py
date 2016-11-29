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
from warnings import warn

from cwrap import BaseCClass, CWrapper
from ert.enkf import ENKF_LIB
from ert.util import StringList
from ert.ecl import EclSum
from ert.ecl import EclGrid
from ert.util import UIReturn


class EclConfig(BaseCClass):
    def __init__(self):
        c_pointer = EclConfig.cNamespace().alloc()
        super(EclConfig, self).__init__(c_pointer)
        
    def free(self):
        EclConfig.cNamespace().free(self)

    #-----------------------------------------------------------------

    def getEclBase(self):
        """ @rtype: str """
        return EclConfig.cNamespace().get_eclbase(self)

    def validateEclBase(self , eclbase_fmt):
        return EclConfig.cNamespace().validate_eclbase(self , eclbase_fmt)

    # Warning: You should probably use the EnkFMain.setEclBase() method to update the Eclipse basename format
    def setEclBase(self , eclbase):
        EclConfig.cNamespace().set_eclbase( self , eclbase )

    #-----------------------------------------------------------------

    def getDataFile(self):
        return EclConfig.cNamespace().get_data_file(self)

    def setDataFile(self , datafile):
        EclConfig.cNamespace().set_data_file(self , datafile)

    def validateDataFile( self , datafile ):
        """ @rtype: UIReturn """
        return EclConfig.cNamespace().validate_data_file( self , datafile )

    #-----------------------------------------------------------------

    def get_gridfile(self):
        """ @rtype: str """
        return EclConfig.cNamespace().get_gridfile(self)

    def set_gridfile(self, gridfile):
        EclConfig.cNamespace().set_gridfile(self, gridfile)
    
    def validateGridFile(self , gridfile):
        return EclConfig.cNamespace().validate_gridfile(self, gridfile)
        
    def get_grid(self):
        warning_message = "The method get_grid() is deprecated. Use getGrid() instead"
        warn(warning_message)
        return self.getGrid( )
    
    def getGrid(self):
        return EclConfig.cNamespace().get_grid(self)

    #-----------------------------------------------------------------

    def getScheduleFile(self):
        return EclConfig.cNamespace().get_schedule_file(self)

    def setScheduleFile(self, schedule_file):
        EclConfig.cNamespace().set_schedule_file(self, schedule_file)

    def validateScheduleFile(self , schedule_file):
        return EclConfig.cNamespace().validate_schedule_file( self , schedule_file )

    def get_sched_file(self):
        return EclConfig.cNamespace().get_sched_file(self)

    #-----------------------------------------------------------------

    def getInitSection(self):
        return EclConfig.cNamespace().get_init_section(self)

    def setInitSection(self, init_section):
        EclConfig.cNamespace().set_init_section(self, init_section)

    def validateInitSection(self, init_section):
        return EclConfig.cNamespace().validate_init_section(self, init_section)

    #-----------------------------------------------------------------

    def getRefcaseName(self):
        return EclConfig.cNamespace().get_refcase_name(self)

    def loadRefcase(self, refcase):
        EclConfig.cNamespace().load_refcase(self, refcase)

    def getRefcase(self):
        """ @rtype: EclSum """
        refcase = EclConfig.cNamespace().get_refcase( self )
        if not refcase is None:
            refcase.setParent(self)

        return refcase


    def validateRefcase(self, refcase):
        return EclConfig.cNamespace().validate_refcase( self , refcase )

    def hasRefcase(self):
        """ @rtype: bool """
        return EclConfig.cNamespace().has_refcase(self)
        
    #-----------------------------------------------------------------

    def get_static_kw_list(self):
        """ @rtype: StringList """
        return EclConfig.cNamespace().get_static_kw_list(self).setParent(self)

    def clear_static_kw(self):
        EclConfig.cNamespace().clear_static_kw(self)

    def add_static_kw(self, kw):
        EclConfig.cNamespace().add_static_kw(self, kw)

    #-----------------------------------------------------------------

    def getDepthUnit(self):
        return EclConfig.cNamespace().get_depth_unit(self)

    def getPressureUnit(self):
        return EclConfig.cNamespace().get_pressure_unit(self)


cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("ecl_config", EclConfig)
cwrapper.registerType("ecl_config_obj", EclConfig.createPythonObject)
cwrapper.registerType("ecl_config_ref", EclConfig.createCReference)


EclConfig.cNamespace().alloc = cwrapper.prototype("c_void_p ecl_config_alloc( )")
EclConfig.cNamespace().free = cwrapper.prototype("void ecl_config_free( ecl_config )")

EclConfig.cNamespace().get_eclbase = cwrapper.prototype("char* ecl_config_get_eclbase( ecl_config )")
EclConfig.cNamespace().validate_eclbase = cwrapper.prototype("ui_return_obj ecl_config_validate_eclbase( ecl_config , char*)")
EclConfig.cNamespace().set_eclbase = cwrapper.prototype("void ecl_config_set_eclbase( ecl_config , char*)")

EclConfig.cNamespace().get_data_file = cwrapper.prototype("char* ecl_config_get_data_file(ecl_config)")
EclConfig.cNamespace().set_data_file = cwrapper.prototype("void ecl_config_set_data_file(ecl_config , char*)")
EclConfig.cNamespace().validate_data_file = cwrapper.prototype("ui_return_obj ecl_config_validate_data_file(ecl_config , char*)")

EclConfig.cNamespace().get_gridfile = cwrapper.prototype("char* ecl_config_get_gridfile(ecl_config)")
EclConfig.cNamespace().set_gridfile = cwrapper.prototype("void ecl_config_set_grid(ecl_config, char*)")
EclConfig.cNamespace().validate_gridfile = cwrapper.prototype("ui_return_obj ecl_config_validate_grid(ecl_config, char*)")
EclConfig.cNamespace().get_grid = cwrapper.prototype("ecl_grid_ref ecl_config_get_grid(ecl_config)")  #todo: fix return type!!!

EclConfig.cNamespace().get_schedule_file = cwrapper.prototype("char* ecl_config_get_schedule_file(ecl_config)")
EclConfig.cNamespace().set_schedule_file = cwrapper.prototype("void ecl_config_set_schedule_file(ecl_config, char*)")
EclConfig.cNamespace().validate_schedule_file = cwrapper.prototype("ui_return_obj ecl_config_validate_schedule_file(ecl_config, char*)")
EclConfig.cNamespace().get_sched_file = cwrapper.prototype("c_void_p ecl_config_get_sched_file(ecl_config)") #todo: fix return type!!!

EclConfig.cNamespace().get_init_section = cwrapper.prototype("char* ecl_config_get_init_section(ecl_config)")
EclConfig.cNamespace().set_init_section = cwrapper.prototype("void ecl_config_set_init_section(ecl_config, char*)")
EclConfig.cNamespace().validate_init_section = cwrapper.prototype("ui_return_obj ecl_config_validate_init_section(ecl_config, char*)")

EclConfig.cNamespace().get_refcase_name = cwrapper.prototype("char* ecl_config_get_refcase_name(ecl_config)")
EclConfig.cNamespace().get_refcase = cwrapper.prototype("ecl_sum_ref ecl_config_get_refcase(ecl_config)")   #todo: fix return type!!!
EclConfig.cNamespace().load_refcase = cwrapper.prototype("void ecl_config_load_refcase(ecl_config, char*)")
EclConfig.cNamespace().validate_refcase = cwrapper.prototype("ui_return_obj ecl_config_validate_refcase(ecl_config, char*)")
EclConfig.cNamespace().has_refcase = cwrapper.prototype("bool ecl_config_has_refcase(ecl_config)")

EclConfig.cNamespace().get_static_kw_list = cwrapper.prototype("stringlist_ref ecl_config_get_static_kw_list(ecl_config)")
EclConfig.cNamespace().clear_static_kw = cwrapper.prototype("void ecl_config_clear_static_kw(ecl_config)")
EclConfig.cNamespace().add_static_kw = cwrapper.prototype("void ecl_config_add_static_kw(ecl_config, char*)")

EclConfig.cNamespace().get_depth_unit = cwrapper.prototype("char* ecl_config_get_depth_unit(ecl_config)")
EclConfig.cNamespace().get_pressure_unit = cwrapper.prototype("char* ecl_config_get_pressure_unit(ecl_config)")

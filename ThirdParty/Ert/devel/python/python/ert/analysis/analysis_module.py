#  Copyright (C) 2013  Statoil ASA, Norway.
#   
#  The file 'analysis_module.py' is part of ERT - Ensemble based Reservoir Tool.
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

from ert.cwrap import BaseCClass, CWrapper
from ert.analysis import ANALYSIS_LIB
from ert.util.rng import RandomNumberGenerator


class AnalysisModule(BaseCClass):
    def __init__(self, rng, user_name, lib_name):
        c_ptr = AnalysisModule.cNamespace().alloc_external(rng, user_name, lib_name)
        super(AnalysisModule , self).__init__(c_ptr)

    def free(self):
        AnalysisModule.cNamespace().free(self)

    def getLibName(self):
        return AnalysisModule.cNamespace().get_lib_name(self)

    def getInternal(self):
        return AnalysisModule.cNamespace().get_module_internal(self)

    def setVar(self, var_name, string_value):
        return AnalysisModule.cNamespace().set_var(self, var_name, string_value)

    def getTableName(self):
        return AnalysisModule.cNamespace().get_table_name(self)

    def getName(self):
        return AnalysisModule.cNamespace().get_name(self)

    def checkOption(self, flag):
        return AnalysisModule.cNamespace().check_option(self, flag)

    def hasVar(self, var):
        return AnalysisModule.cNamespace().has_var(self, var)

    def getDouble(self, var):
        return AnalysisModule.cNamespace().get_double(self, var)

    def getInt(self, var):
        return AnalysisModule.cNamespace().get_int(self, var)

    def getBool(self, var):
        return AnalysisModule.cNamespace().get_bool(self, var)

    def getStr(self, var):
        test = AnalysisModule.cNamespace().get_str(self, var)
        return str(test)

    def initUpdate(self , mask , S , R , dObs , E , D):
        print "Running initUpdate"
        AnalysisModule.cNamespace().init_update(self , mask , S , R , dObs , E , D )

    def updateA(self , A , S , R , dObs , E , D):
        print "Running updateA"
        AnalysisModule.cNamespace().updateA(self , A , S , R , dObs , E , D )

    def initX(self , X , A , S , R , dObs , E , D):
        AnalysisModule.cNamespace().init_update(self , X , A , S , R , dObs , E , D )
        

cwrapper = CWrapper(ANALYSIS_LIB)
cwrapper.registerType("analysis_module", AnalysisModule)
cwrapper.registerType("analysis_module_obj", AnalysisModule.createPythonObject)
cwrapper.registerType("analysis_module_ref", AnalysisModule.createCReference)

AnalysisModule.cNamespace().alloc_external      = cwrapper.prototype("c_void_p analysis_module_alloc_external(rng, char*, char*)")
AnalysisModule.cNamespace().free                = cwrapper.prototype("void analysis_module_free(analysis_module)")
AnalysisModule.cNamespace().get_lib_name        = cwrapper.prototype("char* analysis_module_get_lib_name(analysis_module)")
AnalysisModule.cNamespace().get_module_internal = cwrapper.prototype("bool analysis_module_internal(analysis_module)")
AnalysisModule.cNamespace().set_var             = cwrapper.prototype("bool analysis_module_set_var(analysis_module, char*, char*)")
AnalysisModule.cNamespace().get_table_name      = cwrapper.prototype("char* analysis_module_get_table_name(analysis_module)")
AnalysisModule.cNamespace().get_name            = cwrapper.prototype("char* analysis_module_get_name(analysis_module)")
AnalysisModule.cNamespace().check_option        = cwrapper.prototype("bool analysis_module_check_option(analysis_module, long)")
AnalysisModule.cNamespace().has_var             = cwrapper.prototype("bool analysis_module_has_var(analysis_module, char*)")
AnalysisModule.cNamespace().get_double          = cwrapper.prototype("double analysis_module_get_double(analysis_module, char*)")
AnalysisModule.cNamespace().get_int             = cwrapper.prototype("int analysis_module_get_int(analysis_module, char*)")
AnalysisModule.cNamespace().get_bool             = cwrapper.prototype("bool analysis_module_get_bool(analysis_module, char*)")
AnalysisModule.cNamespace().get_str             = cwrapper.prototype("char* analysis_module_get_ptr(analysis_module, char*)")

AnalysisModule.cNamespace().init_update         = cwrapper.prototype("void analysis_module_init_update(analysis_module, bool_vector , matrix , matrix , matrix , matrix, matrix)")
AnalysisModule.cNamespace().updateA             = cwrapper.prototype("void analysis_module_updateA(analysis_module, matrix , matrix ,  matrix , matrix, matrix, matrix)")
AnalysisModule.cNamespace().initX               = cwrapper.prototype("void analysis_module_initX(analysis_module, matrix , matrix , matrix , matrix , matrix, matrix, matrix)")









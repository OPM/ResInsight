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

from cwrap import BaseCClass
from ert.util.rng import RandomNumberGenerator
from ert.analysis import AnalysisPrototype

from ert.util import Matrix

class AnalysisModule(BaseCClass):
    TYPE_NAME = "analysis_module"
    
    _alloc_external      = AnalysisPrototype("void* analysis_module_alloc_external(rng, char*)" , bind = False)
    _alloc_internal      = AnalysisPrototype("void* analysis_module_alloc_internal(rng, char*)" , bind = False)
    _free                = AnalysisPrototype("void analysis_module_free(analysis_module)")
    _get_lib_name        = AnalysisPrototype("char* analysis_module_get_lib_name(analysis_module)")
    _get_module_internal = AnalysisPrototype("bool analysis_module_internal(analysis_module)")
    _set_var             = AnalysisPrototype("bool analysis_module_set_var(analysis_module, char*, char*)")
    _get_table_name      = AnalysisPrototype("char* analysis_module_get_table_name(analysis_module)")
    _get_name            = AnalysisPrototype("char* analysis_module_get_name(analysis_module)")
    _check_option        = AnalysisPrototype("bool analysis_module_check_option(analysis_module, long)")
    _has_var             = AnalysisPrototype("bool analysis_module_has_var(analysis_module, char*)")
    _get_double          = AnalysisPrototype("double analysis_module_get_double(analysis_module, char*)")
    _get_int             = AnalysisPrototype("int analysis_module_get_int(analysis_module, char*)")
    _get_bool            = AnalysisPrototype("bool analysis_module_get_bool(analysis_module, char*)")
    _get_str             = AnalysisPrototype("char* analysis_module_get_ptr(analysis_module, char*)")
    _init_update         = AnalysisPrototype("void analysis_module_init_update(analysis_module, bool_vector , matrix , matrix , matrix , matrix, matrix)")
    _updateA             = AnalysisPrototype("void analysis_module_updateA(analysis_module, matrix , matrix ,  matrix , matrix, matrix, matrix, void*)")
    _initX               = AnalysisPrototype("void analysis_module_initX(analysis_module, matrix , matrix , matrix , matrix , matrix, matrix, matrix)")


    # The VARIABLE_NAMES field is a completly broken special case
    # which only applies to the rml module.
    VARIABLE_NAMES = {
        "LAMBDA0": {"type": float, "description": "Initial Lambda"},
        "USE_PRIOR": {"type": bool, "description": "Use both Prior and Observation Variability"},
        "LAMBDA_REDUCE": {"type": float, "description": "Lambda Reduction Factor"},
        "LAMBDA_INCREASE": {"type": float, "description": "Lambda Incremental Factor"},
        "LAMBDA_MIN": {"type": float, "description": "Minimum Lambda"},
        "LOG_FILE": {"type": str, "description": "Log File"},
        "CLEAR_LOG": {"type": bool, "description": "Clear Existing Log File"},
        "LAMBDA_RECALCULATE": {"type": bool, "description": "Recalculate Lambda after each Iteration"},
        "ENKF_TRUNCATION": {"type": float, "description": "Singular value truncation"},
        "ENKF_NCOMP": {"type": int, "description": "ENKF_NCOMP"},
        "CV_NFOLDS": {"type": int, "description": "CV_NFOLDS"},
        "FWD_STEP_R2_LIMIT": {"type": float, "description": "FWD_STEP_R2_LIMIT"},
        "CV_PEN_PRESS": {"type": bool, "description": "CV_PEN_PRESS"}
    }

    def __init__(self, rng , name = None , lib_name = None):
        if name is None and lib_name is None:
            raise ValueError("Must supply exactly one of lib or lib_name")

        if name and lib_name:
            raise ValueError("Must supply exactly one of name or lib_name")

        if lib_name:
            c_ptr = self._alloc_external(rng, lib_name )
        else:
            c_ptr = self._alloc_internal( rng , name )
            if not c_ptr:
                raise KeyError("Failed to load internal module:%s" % name)
            
        super(AnalysisModule, self).__init__(c_ptr)

        
    def getVariableNames(self):
        """ @rtype: list of str """
        items = []
        for name in AnalysisModule.VARIABLE_NAMES:
            if self.hasVar(name):
                items.append(name)
        return items
    
    def getVariableValue(self, name):
        """ @rtype: int or float or bool or str """
        variable_type = self.getVariableType(name)
        if variable_type == float:
            return self.getDouble(name)
        elif variable_type == bool:
            return self.getBool(name)
        elif variable_type == str:
            return self.getStr(name)
        elif variable_type == int:
            return self.getInt(name)
    
    def getVariableType(self, name):
        """ :rtype: type """
        return AnalysisModule.VARIABLE_NAMES[name]["type"]

    def getVariableDescription(self, name):
        """ :rtype: str """
        return AnalysisModule.VARIABLE_NAMES[name]["description"]

    def getVar(self, name):
        return self.getVariableValue( name )
    
    def free(self):
        self._free( )

    def __repr__(self):
        nm = self.name()
        tn = self.getTableName()
        ln = self.getLibName()
        mi = 'internal' if self.getInternal() else 'external'
        ad = self._ad_str()
        fmt = 'AnalysisModule(name = %s, table = %s, lib = %s, %s) %s'
        return fmt % (nm, tn, ln, mi, ad)

    def getLibName(self):
        return self._get_lib_name( )

    def getInternal(self):
        return self._get_module_internal( )

    def __assertVar(self , var_name):
        if not self.hasVar(var_name):
            raise KeyError("Module does not support key:%s" % var_name)
    
    def setVar(self, var_name, value):
        self.__assertVar( var_name )
        string_value = str(value)
        return self._set_var(var_name, string_value)

    
    def getTableName(self):
        return self._get_table_name( )

    def getName(self):
        """ :rtype: str """
        return self.name()

    def name(self):
        return self._get_name( )

    def checkOption(self, flag):
        return self._check_option(flag)

    def hasVar(self, var):
        """ :rtype: bool """
        return self._has_var(var)

    def getDouble(self, var):
        """ :rtype: float """
        self.__assertVar( var )
        return self._get_double(var)

    def getInt(self, var):
        """ :rtype: int """
        self.__assertVar( var )
        return self._get_int(var)

    def getBool(self, var):
        """ :rtype: bool """
        self.__assertVar( var )
        return self._get_bool(var)

    def getStr(self, var):
        """ :rtype: str """
        self.__assertVar( var )
        return self._get_str(var)

    
    def initUpdate(self, mask, S, R, dObs, E, D):
        self._init_update(mask, S, R, dObs, E, D)

        
    def updateA(self, A, S, R, dObs, E, D):
        self._updateA(A, S, R, dObs, E, D, None)

        
    def initX(self, A, S, R, dObs, E, D):
        X = Matrix( A.columns() , A.columns())
        self._initX(X, A, S, R, dObs, E, D)
        return X

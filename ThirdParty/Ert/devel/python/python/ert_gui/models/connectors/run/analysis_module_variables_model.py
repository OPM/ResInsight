#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'analysis_module_variables_model.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.analysis.analysis_module import AnalysisModule
from ert_gui.models import ErtConnector
from ert_gui.models.connectors import EnsembleSizeModel

class AnalysisModuleVariablesModel(ErtConnector):

    def __init__(self):
        super(AnalysisModuleVariablesModel, self).__init__()
        self.__variable_names = {
            "LAMBDA0": {"type": float, "min": -1, "max": 10000000000000, "step":1.0, "labelname":"Initial Lambda", "pos":0},
            "USE_PRIOR": {"type": bool, "labelname":"Use both Prior and Observation Variability", "pos":1},
            "LAMBDA_REDUCE": {"type": float, "min": 0, "max": 1, "step":0.1, "labelname":"Lambda Reduction Factor", "pos":2},
            "LAMBDA_INCREASE": {"type": float, "min": 1, "max": 10, "step":0.1, "labelname":"Lambda Incremental Factor", "pos":3},
            "LAMBDA_MIN": {"type": float, "min": 0, "max": 10, "step":0.1, "labelname":"Minimum Lambda", "pos":4},
            "LOG_FILE": {"type": str, "labelname":"Log File", "pos":5},
            "CLEAR_LOG": {"type": bool, "labelname":"Clear Existing Log File", "pos":6},
            "LAMBDA_RECALCULATE": {"type": bool, "labelname":"Recalculate Lambda after each Iteration", "pos":7},
            "ENKF_TRUNCATION" :{"type": float, "min": 0, "max": 1, "step":0.1, "labelname":"Singular value truncation", "pos":9},
            "ENKF_NCOMP": {"type": int, "min": -1, "max": 10, "step":1.0, "labelname":"ENKF_NCOMP", "pos":10},
            "CV_NFOLDS": {"type": int, "min": 2, "max": EnsembleSizeModel().getValue() - 1, "step":1.0, "labelname":"CV_NFOLDS", "pos":11},
            "FWD_STEP_R2_LIMIT":{"type": float, "min": -1, "max": 100, "step":1.0, "labelname":"FWD_STEP_R2_LIMIT", "pos":12},
            "CV_PEN_PRESS": {"type": bool, "labelname":"CV_PEN_PRESS", "pos":13}
        }


    def getVariableNames(self, analysis_module_name):
        """ @rtype: list of str """
        analysis_module = self.ert().analysisConfig().getModule(analysis_module_name)
        assert isinstance(analysis_module, AnalysisModule)
        items = []
        for name in self.__variable_names:
            if analysis_module.hasVar(name):
                items.append(name)
        return items

    def getVariableType(self, name):
        return self.__variable_names[name]["type"]

    def getVariableMaximumValue(self, name):
        return self.__variable_names[name]["max"]

    def getVariableMinimumValue(self, name):
        return self.__variable_names[name]["min"]

    def getVariableStepValue(self, name):
        return self.__variable_names[name]["step"]

    def getVariableLabelName(self, name):
        return self.__variable_names[name]["labelname"]

    def getVariablePosition(self, name):
        return self.__variable_names[name]["pos"]



    def setVariableValue(self, analysis_module_name, name, value):
        analysis_module = self.ert().analysisConfig().getModule(analysis_module_name)
        result = analysis_module.setVar(name,str(value))



    def getVariableValue(self, analysis_module_name, name):
        """ @rtype: int or float or bool or str """
        analysis_module = self.ert().analysisConfig().getModule(analysis_module_name)
        variable_type = self.getVariableType(name)
        if variable_type == float:
            return analysis_module.getDouble(name)
        elif variable_type == bool:
            return analysis_module.getBool(name)
        elif variable_type == str:
            return analysis_module.getStr(name)
        elif variable_type == int:
            return  analysis_module.getInt(name)



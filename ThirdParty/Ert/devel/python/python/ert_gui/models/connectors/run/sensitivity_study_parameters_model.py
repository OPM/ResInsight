'''
Created on Aug 22, 2014

@author: perroe
'''

from ert_gui.models import ErtConnector
from ert.enkf.enums.ert_impl_type_enum import ErtImplType
from ert.enkf.plot import EnsembleGenKWFetcher


class SensitivityStudyParametersModel(ErtConnector):

    def __init__(self):
        super(SensitivityStudyParametersModel, self).__init__()
        self.__params = None
        self.__is_included = {}
        self.__constant_val = {}
        self.__parameter_type = {}


    def setIsIncluded(self, parameter_name, is_included):
        self.__is_included[parameter_name] = is_included

    def setConstantValue(self, parameter_name, constant_value):
        self.__constant_val[parameter_name] = constant_value

    def getParameters(self):

        if self.__params is None:
            self.__params = []
            for key in EnsembleGenKWFetcher(self.ert()).fetchSupportedKeys():
                self.__params.append(key)
                self.__is_included[key] = True
                self.__constant_val[key] = 0.5
                self.__parameter_type[key] = ErtImplType.GEN_KW

            for key in self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.FIELD):
                self.__params.append(key)
                self.__is_included[key] = True
                self.__constant_val[key] = None
                self.__parameter_type[key] = ErtImplType.FIELD

            for key in self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_DATA):
                self.__params.append(key)
                self.__is_included[key] = True
                self.__constant_val[key] = None
                self.__parameter_type[key] = ErtImplType.GEN_DATA
                
        return self.__params

    def getIsIncluded(self, parameter_name):
        return self.__is_included[parameter_name]

    def getConstantValue(self, parameter_name):
        return self.__constant_val[parameter_name]

    def getParameterType(self, parameter_name):
        return self.__parameter_type[parameter_name]


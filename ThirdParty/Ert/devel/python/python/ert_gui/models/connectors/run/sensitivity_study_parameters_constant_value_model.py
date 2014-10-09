'''
Created on Aug 26, 2014

@author: perroe
'''

from ert_gui.models import ErtConnector
from ert_gui.models.mixins import BasicModelMixin

class SensivityStudyParametersConstantValueModel(ErtConnector, BasicModelMixin):
    '''
    Model used for setting if a parameter is included in the sensitivity study.
    '''

    def __init__(self, parameter_name, sensitivity_study_parameters_model):
        '''
        Constructor
        '''
        self.__parameter_name = parameter_name
        self.__model = sensitivity_study_parameters_model


    def getValue(self):
        """ @rtype: float """
        return self.__model.getConstantValue(self.__parameter_name)


    def setValue(self, constant_value):
        self.__model.setConstantValue(self.__parameter_name, constant_value)

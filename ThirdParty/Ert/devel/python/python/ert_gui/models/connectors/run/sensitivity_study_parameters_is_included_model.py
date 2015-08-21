'''
Created on Aug 26, 2014

@author: perroe
'''

from ert_gui.models import ErtConnector
from ert_gui.models.mixins import BooleanModelMixin

class SensivityStudyParametersIsIncludedModel(ErtConnector, BooleanModelMixin):
    '''
    Model used for setting if a parameter is included in the sensitivity study.
    '''

    def __init__(self, parameter_name, sensitivity_study_parameters_model):
        '''
        Constructor
        '''
        self.__parameter_name = parameter_name
        self.__model = sensitivity_study_parameters_model


    def isTrue(self):
        """ @rtype: bool """
        return self.__model.getIsIncluded(self.__parameter_name)


    def setState(self, is_included):
        self.__model.setIsIncluded(self.__parameter_name, is_included)

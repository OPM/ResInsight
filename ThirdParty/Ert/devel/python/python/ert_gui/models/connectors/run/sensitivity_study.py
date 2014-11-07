'''
Created on 9. juli 2014

@author: perroe
'''

from ert_gui.models.connectors.run import BaseRunModel, SensitivityTargetCaseFormatModel 
from ert_gui.models.connectors.run.sensitivity_study_parameters_model import SensitivityStudyParametersModel
#from ert_gui.models.mixins.run_model import ErtRunError

class SensitivityStudy(BaseRunModel):
    '''
    A sensitivity study, running experiments varying a parameter at a time.
    '''

    def __init__(self):
        super(SensitivityStudy, self).__init__(name = "Sensitivity Study",
                                               phase_count = 1)

    def createTargetCaseFileSystem(self, parameter_name):
        tmp_param_name = parameter_name
        tmp_param_name.replace(":", "-")
        target_case_format = SensitivityTargetCaseFormatModel().getValue()
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_format % tmp_param_name)
        return target_fs

    def runSimulations(self):
        '''
        Run all the individual simulations
        '''
        parameter_model = SensitivityStudyParametersModel()
        parameters_used = [p for p in parameter_model.getParameters() if parameter_model.getIsIncluded(p)]
        self.setPhaseCount(len(parameters_used))

        phase = 0
        
        for parameter in parameters_used:
            phase += 1
            print "parameter {0} of {1}: {2}".format(phase, len(parameters_used), parameter) 
            self.setPhase(phase, "Parameter {0}.".format(parameter))

'''
Created on 9. juli 2014

@author: perroe
'''

from ert_gui.models.connectors.run import BaseRunModel # ActiveRealizationsModel,
#from ert_gui.models.mixins.run_model import ErtRunError

class SensitivityStudy(BaseRunModel):
    '''
    A sensitivity study, running experiments varying a parameter at a time.
    '''

    def __init__(self):
        super(SensitivityStudy, self).__init__(name = "Sensitivity Study",
                                               phase_count = 1)

    def runSimulations(self):
        '''
        Run all the individual simulations
        '''
        phase_count = 1
        self.setPhaseCount(phase_count)

        self.setPhase(phase_count, "Simulations not implemented.")

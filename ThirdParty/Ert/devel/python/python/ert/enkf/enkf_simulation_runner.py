from ert.cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB, EnkfStateType, EnkfFs
from ert.enkf.enums import EnkfInitModeEnum
from ert.util import BoolVector


class EnkfSimulationRunner(BaseCClass):

    def __init__(self, enkf_main):
        assert isinstance(enkf_main, BaseCClass)
        super(EnkfSimulationRunner, self).__init__(enkf_main.from_param(enkf_main).value, parent=enkf_main, is_reference=True)

    def runSimpleStep(self, active_realization_mask, initialization_mode):
        """ @rtype: bool """
        assert isinstance(active_realization_mask, BoolVector)
        assert isinstance(initialization_mode, EnkfInitModeEnum)
        return EnkfSimulationRunner.cNamespace().run_simple_step(self, active_realization_mask, initialization_mode)

    def iterateSmoother(self, iteration_number, target_case_name, active_realization_mask):
        """ @rtype: bool """
        assert isinstance(active_realization_mask, BoolVector)
        return EnkfSimulationRunner.cNamespace().iterate_smoother(self , iteration_number , target_case_name , active_realization_mask)
    
        
    def runEnsembleExperiment(self, active_realization_mask):
        """ @rtype: bool """
        return self.runSimpleStep(active_realization_mask , EnkfInitModeEnum.INIT_CONDITIONAL)

    def runPostWorkflow(self):
        EnkfSimulationRunner.cNamespace().run_post_workflow(self)


    def smootherUpdate(self, target_fs):
        """ @rtype: bool """
        assert isinstance(target_fs, EnkfFs)
        return EnkfSimulationRunner.cNamespace().smoother_update(self, target_fs)


    # def run(self, boolPtr, init_step_parameter, simFrom, state, mode):
    #     #{"ENKF_ASSIMILATION" : 1, "ENSEMBLE_EXPERIMENT" : 2, "ENSEMBLE_PREDICTION" : 3, "INIT_ONLY" : 4, "SMOOTHER" : 5}
    #     if mode == 1:
    #         EnKFMain.cNamespace().run_assimilation(self, boolPtr, init_step_parameter, simFrom, state)
    #
    #     if mode == 2:
    #         EnKFMain.cNamespace().run_exp(self, boolPtr, True, init_step_parameter, simFrom, state, True)
    #
    #     if mode == 4:
    #         EnKFMain.cNamespace().run_exp(self, boolPtr, False, init_step_parameter, simFrom, state , True)
    #
    #     if mode == 5:
    #         EnKFMain.cNamespace().run_smoother(self, "AUTOSMOOTHER", True)

    # def runIteratedEnsembleSmoother(self, last_report_step):
    #     #warn: Remember to select correct analysis module RML
    #     EnKFMain.cNamespace().run_iterated_ensemble_smoother(self, last_report_step)
    #
    # def runOneMoreIteration(self, last_report_step):
    #     #warn: need some way of validating that the case has run
    #     EnKFMain.cNamespace().run_one_more_iteration(self, last_report_step)

cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("enkf_simulation_runner", EnkfSimulationRunner)

EnkfSimulationRunner.cNamespace().run_assimilation  = cwrapper.prototype("void enkf_main_run_assimilation(enkf_simulation_runner, bool_vector, int, int, int)")
EnkfSimulationRunner.cNamespace().run_smoother      = cwrapper.prototype("void enkf_main_run_smoother(enkf_simulation_runner, char*, bool)")

EnkfSimulationRunner.cNamespace().run_simple_step   = cwrapper.prototype("bool enkf_main_run_simple_step(enkf_simulation_runner, bool_vector, enkf_init_mode_enum)")
EnkfSimulationRunner.cNamespace().smoother_update   = cwrapper.prototype("bool enkf_main_smoother_update(enkf_simulation_runner, enkf_fs)")
EnkfSimulationRunner.cNamespace().run_post_workflow = cwrapper.prototype("void enkf_main_run_post_workflow(enkf_simulation_runner)")
EnkfSimulationRunner.cNamespace().iterate_smoother  = cwrapper.prototype("bool enkf_main_iterate_smoother(enkf_simulation_runner, int, char*, bool_vector)")

from ert.enkf.enums import EnkfInitModeEnum, HookRuntime
from ert_gui.models.connectors.run import NumberOfIterationsModel, ActiveRealizationsModel, IteratedAnalysisModuleModel, BaseRunModel
from ert_gui.models.connectors.run.target_case_format_model import TargetCaseFormatModel
from ert_gui.models.mixins import ErtRunError


class IteratedEnsembleSmoother(BaseRunModel):

    def __init__(self):
        super(IteratedEnsembleSmoother, self).__init__(name="Iterated Ensemble Smoother", phase_count=2)

    def setAnalysisModule(self):
        module_name = IteratedAnalysisModuleModel().getCurrentChoice()
        module_load_success = self.ert().analysisConfig().selectModule(module_name)

        if not module_load_success:
            raise ErtRunError("Unable to load analysis module '%s'!" % module_name)

        return self.ert().analysisConfig().getModule(module_name)


    def runAndPostProcess(self, active_realization_mask, phase, phase_count, mode):
        self.setPhase(phase, "Running iteration %d of %d simulation iterations..." % (phase, phase_count - 1), indeterminate=False)

        self.setPhaseName("Pre processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, phase)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        self.setPhaseName("Running forecast...", indeterminate=False)
        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, mode, phase)

        if not success:
            min_realization_count = self.ert().analysisConfig().getMinRealisations()
            success_count = active_realization_mask.count()

            if min_realization_count > success_count:
                raise ErtRunError("Simulation failed! Number of successful realizations less than MIN_REALIZATIONS %d < %d" % (success_count, min_realization_count))
            elif success_count == 0:
                raise ErtRunError("Simulation failed! All realizations failed!")
            #ignore and continue

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )


    def createTargetCaseFileSystem(self, phase):
        target_case_format = TargetCaseFormatModel().getValue()
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_format % phase)
        return target_fs


    def analyzeStep(self, target_fs):
        self.setPhaseName("Analyzing...", indeterminate=True)
        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        success = self.ert().getEnkfSimulationRunner().smootherUpdate(source_fs , target_fs)

        if not success:
            raise ErtRunError("Analysis of simulation failed!")


    def runSimulations(self):
        phase_count = NumberOfIterationsModel().getValue() + 1
        self.setPhaseCount(phase_count)

        analysis_module = self.setAnalysisModule()
        active_realization_mask = ActiveRealizationsModel().getActiveRealizationsMask()

        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        initial_fs = self.createTargetCaseFileSystem(0)

        if not source_fs == initial_fs:
            self.ert().getEnkfFsManager().switchFileSystem(initial_fs)
            self.ert().getEnkfFsManager().initializeCurrentCaseFromExisting(source_fs, 0)

        self.runAndPostProcess(active_realization_mask, 0, phase_count, EnkfInitModeEnum.INIT_CONDITIONAL)
        target_case_format = TargetCaseFormatModel().getValue()
        self.ert().analysisConfig().getAnalysisIterConfig().setCaseFormat( target_case_format )

        analysis_config = self.ert().analysisConfig()
        analysis_iter_config = analysis_config.getAnalysisIterConfig()
        num_retries_per_iteration = analysis_iter_config.getNumRetries()
        num_tries = 0
        current_iteration = 1

        while current_iteration <= NumberOfIterationsModel().getValue() and num_tries < num_retries_per_iteration:
            target_fs = self.createTargetCaseFileSystem(current_iteration)

            pre_analysis_iter_num = analysis_module.getInt("ITER")
            self.analyzeStep(target_fs)
            post_analysis_iter_num = analysis_module.getInt("ITER")

            analysis_success = False
            if  post_analysis_iter_num > pre_analysis_iter_num:
                analysis_success = True

            if analysis_success:
                self.ert().getEnkfFsManager().switchFileSystem(target_fs)
                self.runAndPostProcess(active_realization_mask, current_iteration, phase_count, EnkfInitModeEnum.INIT_NONE)
                num_tries = 0
                current_iteration += 1
            else:
                self.ert().getEnkfFsManager().initializeCurrentCaseFromExisting(target_fs, 0)
                self.runAndPostProcess(active_realization_mask, current_iteration - 1 , phase_count, EnkfInitModeEnum.INIT_NONE)
                num_tries += 1



        if current_iteration == phase_count:
            self.setPhase(phase_count, "Simulations completed.")
        else:
            raise ErtRunError("Iterated Ensemble Smoother stopped: maximum number of iteration retries (%d retries) reached for iteration %d" % (num_retries_per_iteration, current_iteration))



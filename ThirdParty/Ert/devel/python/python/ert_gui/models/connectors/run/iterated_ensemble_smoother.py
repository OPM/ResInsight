from ert.enkf.enums import EnkfInitModeEnum
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


    def runAndPostProcess(self, phase, phase_count,  mode):
        self.setPhase(phase, "Running iteration %d of %d simulation iterations..." % (phase + 1, phase_count), indeterminate=False)

        active_realization_mask = ActiveRealizationsModel().getActiveRealizationsMask()
        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, mode)

        if not success:
            raise ErtRunError("Simulation failed!")

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runPostWorkflow()


    def createTargetCaseFileSystem(self, phase):
        target_case_format = TargetCaseFormatModel().getValue()
        target_fs = self.ert().getEnkfFsManager().mountAlternativeFileSystem(target_case_format % phase, read_only=False, create=True)

        return target_fs


    def analyzeStep(self, target_fs):
        self.setPhaseName("Analyzing...", indeterminate=True)
        success = self.ert().getEnkfSimulationRunner().smootherUpdate(target_fs)

        if not success:
            raise ErtRunError("Analysis of simulation failed!")


    def runSimulations(self):
        iteration_count = NumberOfIterationsModel().getValue()
        phase_count = iteration_count
        self.setPhaseCount(phase_count)

        analysis_module = self.setAnalysisModule()

        analysis_module.setVar("ITER", str(0))
        self.runAndPostProcess(0, phase_count, EnkfInitModeEnum.INIT_CONDITIONAL)

        for phase in range(1, self.phaseCount()):
            target_fs = self.createTargetCaseFileSystem(phase)

            self.analyzeStep(target_fs)

            self.ert().getEnkfFsManager().switchFileSystem(target_fs)

            analysis_module.setVar("ITER", str(phase))
            self.runAndPostProcess(phase, phase_count, EnkfInitModeEnum.INIT_NONE)

        self.setPhase(phase_count, "Simulations completed.")

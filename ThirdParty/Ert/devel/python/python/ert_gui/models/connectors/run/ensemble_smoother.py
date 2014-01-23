from ert.enkf.enums import EnkfInitModeEnum
from ert_gui.models.connectors.run import ActiveRealizationsModel, TargetCaseModel, AnalysisModuleModel, BaseRunModel
from ert_gui.models.mixins import ErtRunError


class EnsembleSmoother(BaseRunModel):

    def __init__(self):
        super(EnsembleSmoother, self).__init__(name="Ensemble Smoother", phase_count=2)

    def setAnalysisModule(self):
        module_name = AnalysisModuleModel().getCurrentChoice()
        module_load_success = self.ert().analysisConfig().selectModule(module_name)

        if not module_load_success:
            raise ErtRunError("Unable to load analysis module '%s'!" % module_name)


    def runSimulations(self):
        self.setPhase(0, "Running simulations...", indeterminate=False)

        self.setAnalysisModule()

        active_realization_mask = ActiveRealizationsModel().getActiveRealizationsMask()

        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_CONDITIONAL)

        if not success:
            raise ErtRunError("Simulation failed!")

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runPostWorkflow()

        self.setPhaseName("Analyzing...", indeterminate=True)

        target_case_name = TargetCaseModel().getValue()
        target_fs = self.ert().getEnkfFsManager().mountAlternativeFileSystem(target_case_name, read_only=False, create=True)

        success = self.ert().getEnkfSimulationRunner().smootherUpdate(target_fs)

        if not success:
            raise ErtRunError("Analysis of simulation failed!")

        self.setPhase(1, "Running simulations...", indeterminate=False)

        self.ert().getEnkfFsManager().switchFileSystem(target_fs)
        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_NONE)

        if not success:
            raise ErtRunError("Simulation failed!")

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runPostWorkflow()

        self.setPhase(2, "Simulations completed.")



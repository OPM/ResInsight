from ert.enkf.enums import EnkfInitModeEnum
from ert_gui.models.connectors.run import ActiveRealizationsModel, TargetCaseModel, AnalysisModuleModel, BaseRunModel
from ert_gui.models.mixins import ErtRunError
from ert.enkf.enums import HookRuntime


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

        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_CONDITIONAL , 0)

        if not success:
            min_realization_count = self.ert().analysisConfig().getMinRealisations()
            success_count = active_realization_mask.count()

            if min_realization_count > success_count:
                raise ErtRunError("Simulation failed! Number of successful realizations less than MIN_REALIZATIONS %d < %d" % (success_count, min_realization_count))
            elif success_count == 0:
                raise ErtRunError("Simulation failed! All realizations failed!")
            #else ignore and continue


        
        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )
        
        self.setPhaseName("Analyzing...", indeterminate=True)

        target_case_name = TargetCaseModel().getValue()
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)
        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        success = self.ert().getEnkfSimulationRunner().smootherUpdate(source_fs , target_fs)

        if not success:
            raise ErtRunError("Analysis of simulation failed!")

        self.setPhase(1, "Running simulations...", indeterminate=False)

        self.ert().getEnkfFsManager().switchFileSystem(target_fs)
        success = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_NONE, 1)

        if not success:
            raise ErtRunError("Simulation failed!")

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )

        self.setPhase(2, "Simulations completed.")



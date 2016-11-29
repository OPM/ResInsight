from ert.enkf.enums import EnkfInitModeEnum
from ert.enkf.enums import HookRuntime
from ert_gui.simulation.models import BaseRunModel, ErtRunError


class EnsembleSmoother(BaseRunModel):

    def __init__(self):
        super(EnsembleSmoother, self).__init__(name="Ensemble Smoother", phase_count=2)

    def setAnalysisModule(self, module_name):
        module_load_success = self.ert().analysisConfig().selectModule(module_name)

        if not module_load_success:
            raise ErtRunError("Unable to load analysis module '%s'!" % module_name)


    def runSimulations(self, arguments):
        self.setPhase(0, "Running simulations...", indeterminate=False)

        self.setAnalysisModule(arguments["analysis_module"])
        active_realization_mask = arguments["active_realizations"]

        self.setPhaseName("Pre processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, 0)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        self.setPhaseName("Running forecast...", indeterminate=False)
        num_successful_realizations = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_CONDITIONAL , 0)

        self.checkHaveSufficientRealizations(num_successful_realizations)

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )

        self.setPhaseName("Analyzing...")

        target_case_name = arguments["target_case"]
        target_fs = self.ert().getEnkfFsManager().getFileSystem(target_case_name)
        source_fs = self.ert().getEnkfFsManager().getCurrentFileSystem()
        
        es_update = self.ert().getESUpdate( ) 
        success = es_update.smootherUpdate(source_fs, target_fs)
        if not success:
            raise ErtRunError("Analysis of simulation failed!")

        self.setPhase(1, "Running simulations...")
        self.ert().getEnkfFsManager().switchFileSystem(target_fs)
        
        self.setPhaseName("Pre processing...")
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, 1)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        self.setPhaseName("Running forecast...", indeterminate=False)

        num_successful_realizations = self.ert().getEnkfSimulationRunner().runSimpleStep(active_realization_mask, EnkfInitModeEnum.INIT_NONE, 1)

        self.checkHaveSufficientRealizations(num_successful_realizations)

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )

        self.setPhase(2, "Simulations completed.")

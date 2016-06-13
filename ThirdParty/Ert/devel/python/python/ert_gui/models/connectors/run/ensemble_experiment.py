from ert_gui.models.connectors.run import ActiveRealizationsModel, BaseRunModel
from ert_gui.models.mixins.run_model import ErtRunError
from ert.enkf.enums import HookRuntime


class EnsembleExperiment(BaseRunModel):

    def __init__(self):
        super(EnsembleExperiment, self).__init__("Ensemble Experiment")

    def runSimulations(self):
        self.setPhase(0, "Running simulations...", indeterminate=False)
        active_realization_mask = ActiveRealizationsModel().getActiveRealizationsMask()

        self.setPhaseName("Pre processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, 0)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        self.setPhaseName("Running ensemble experiment...", indeterminate=False)

        success = self.ert().getEnkfSimulationRunner().runEnsembleExperiment(active_realization_mask)

        if not success:
            raise ErtRunError("Simulation failed!")

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )

        self.setPhase(1, "Simulations completed.") # done...




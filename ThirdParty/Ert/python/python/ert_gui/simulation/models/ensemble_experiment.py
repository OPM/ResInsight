from ert.enkf.enums import HookRuntime
from ert_gui.simulation.models import BaseRunModel, ErtRunError


class EnsembleExperiment(BaseRunModel):

    def __init__(self):
        super(EnsembleExperiment, self).__init__("Ensemble Experiment")

    def runSimulations(self, arguments):
        self.setPhase(0, "Running simulations...", indeterminate=False)
        active_realization_mask = arguments["active_realizations"]

        self.setPhaseName("Pre processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().createRunPath(active_realization_mask, 0)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.PRE_SIMULATION )

        self.setPhaseName("Running ensemble experiment...", indeterminate=False)

        num_successful_realizations = self.ert().getEnkfSimulationRunner().runEnsembleExperiment(active_realization_mask)

        self.checkHaveSufficientRealizations(num_successful_realizations)

        self.setPhaseName("Post processing...", indeterminate=True)
        self.ert().getEnkfSimulationRunner().runWorkflows( HookRuntime.POST_SIMULATION )

        self.setPhase(1, "Simulations completed.") # done...




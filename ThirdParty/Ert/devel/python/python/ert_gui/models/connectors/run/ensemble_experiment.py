from ert_gui.models import ErtConnector
from ert_gui.models.connectors.ensemble_resizer import EnsembleSizeModel
from ert_gui.models.connectors.run import RunMembersModel
from ert_gui.models.mixins import ButtonModelMixin, RunModelMixin


class EnsembleExperiment(ErtConnector, RunModelMixin, ButtonModelMixin):

    def startSimulations(self):
        selected_members = [int(member) for member in RunMembersModel().getSelectedItems()]
        total_member_count = EnsembleSizeModel().getSpinnerValue()

        self.ert().runEnsembleExperiment(selected_members, total_member_count)

    def killAllSimulations(self):
        job_queue = self.ert().siteConfig().getJobQueue()
        job_queue.killAllJobs()


    def buttonTriggered(self):
        self.startSimulations()
        self.observable().notify(ButtonModelMixin.BUTTON_TRIGGERED_EVENT)

    def getButtonName(self):
        return "Run"

    def buttonIsEnabled(self):
        return True

    def __str__(self):
        return "Ensemble Experiment"




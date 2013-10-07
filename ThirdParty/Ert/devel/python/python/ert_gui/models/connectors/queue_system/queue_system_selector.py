from ert_gui.models import ErtConnector
from ert_gui.models.mixins import ChoiceModelMixin


class QueueSystemSelector(ErtConnector, ChoiceModelMixin):

    def getChoices(self):
        return ["LSF", "RSH", "LOCAL"]

    def getCurrentChoice(self):
        return self.ert().siteConfig().getQueueName().upper()

    def setCurrentChoice(self, queue_type_name):
        self.ert().siteConfig().setJobQueue(str(queue_type_name))
        self.observable().notify(ChoiceModelMixin.CURRENT_CHOICE_CHANGED_EVENT)




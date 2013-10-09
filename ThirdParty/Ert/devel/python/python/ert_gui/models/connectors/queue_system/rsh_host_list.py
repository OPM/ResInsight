from ert_gui.models import ErtConnector
from ert_gui.models.mixins.dictionary_model import DictionaryModelMixin


class RshHostListModel(ErtConnector, DictionaryModelMixin):

    def getDictionary(self):
        host_list = self.ert().siteConfig().getRshHostList()
        return host_list

    def addKey(self, key):
        self.setValueForKey(key, 1)

    def setValueForKey(self, key, value):
        try:
            max_running = int(value)
        except ValueError:
            max_running = 1

        self.ert().siteConfig().addRshHost(key, max_running)

        self.observable().notify(self.DICTIONARY_CHANGED_EVENT)

    def removeKey(self, key_to_remove):
        hosts = self.getDictionary()

        self.ert().siteConfig().clearRshHostList()

        for key in hosts:
            if not key == key_to_remove:
                self.ert().siteConfig().addRshHost(key, hosts[key])

        self.observable().notify(self.DICTIONARY_CHANGED_EVENT)







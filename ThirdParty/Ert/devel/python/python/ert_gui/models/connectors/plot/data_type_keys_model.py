from ert.enkf.enums import ErtImplType, EnkfObservationImplementationType
from ert_gui.models import ErtConnector
from ert_gui.models.mixins.list_model import ListModelMixin


class DataTypeKeysModel(ErtConnector, ListModelMixin):

    def __init__(self):
        self.__keys = None
        self.__observation_keys = None
        self.__summary_keys = None
        super(DataTypeKeysModel, self).__init__()


    def getAllSummaryKeys(self):
        """ @rtype: list of str """
        if self.__summary_keys is None:
            keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)
            self.__summary_keys = sorted([key for key in keys], key=lambda k : k.lower())

        return self.__summary_keys


    def getAllObservationSummaryKeys(self):
        """ @rtype: list of str """
        if self.__observation_keys is None:
            self.__observation_keys = [key for key in self.getAllSummaryKeys() if self.isObservationKey(key)]

        return self.__observation_keys


    def getList(self):
        """ @rtype: list of str """
        self.__keys = self.__keys or self.getAllSummaryKeys()
        return self.__keys


    def isObservationKey(self, item):
        """ @rtype: bool """
        if self.__observation_keys is None:
            return len(self.ert().ensembleConfig().getNode(item).getObservationKeys()) > 0
        else:
            return item in self.__observation_keys


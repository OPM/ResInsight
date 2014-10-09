from ert.enkf.enums import ErtImplType, EnkfObservationImplementationType
from ert.enkf.plot import BlockObservationDataFetcher, EnsembleGenKWFetcher, EnsembleGenDataFetcher, ObservationGenDataFetcher, PcaDataFetcher
from ert_gui.models import ErtConnector
from ert_gui.models.mixins.list_model import ListModelMixin


class DataTypeKeysModel(ErtConnector, ListModelMixin):

    def __init__(self):
        self.__keys = None
        self.__observation_keys = None
        self.__block_observation_keys = None
        self.__summary_keys = None
        self.__gen_kw_keys = None
        self.__gen_data_keys = None

        self.__custom_pca_keys = {"PCA:All": PcaDataFetcher(self.ert()).getAllObsKeys()}
        super(DataTypeKeysModel, self).__init__()


    def getAllKeys(self):
        """ @rtype: list of str """
        keys = self.getAllSummaryKeys() + self.getAllBlockObservationKeys() + self.getAllGenKWKeys() + self.getAllGenDataKeys() + self.getCustomPcaKeys()
        # return sorted([key for key in keys], key=lambda k : k.lower())
        return keys

    def getAllGenKWKeys(self):
        """ @rtype: list of str """
        if self.__gen_kw_keys is None:
            keys = EnsembleGenKWFetcher(self.ert()).getSupportedKeys()
            self.__gen_kw_keys = sorted(keys, key=lambda k : k.lower())

        return self.__gen_kw_keys

    def getAllGenDataKeys(self):
        """ @rtype: list of str """
        if self.__gen_data_keys is None:
            keys = ObservationGenDataFetcher(self.ert()).getSupportedKeys()
            self.__gen_data_keys = sorted(keys, key=lambda k : k.lower())

        return self.__gen_data_keys

    def getAllSummaryKeys(self):
        """ @rtype: list of str """
        if self.__summary_keys is None:
            keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)
            self.__summary_keys = sorted([key for key in keys], key=lambda k : k.lower())

        return self.__summary_keys

    def getAllBlockObservationKeys(self):
        """ @rtype: list of str """
        if self.__block_observation_keys is None:
            keys = BlockObservationDataFetcher(self.ert()).getSupportedKeys()
            self.__block_observation_keys = sorted(keys, key=lambda k : k.lower())

        return self.__block_observation_keys

    def getAllObservationSummaryKeys(self):
        """ @rtype: list of str """
        if self.__observation_keys is None:
            self.__observation_keys = [key for key in self.getAllSummaryKeys() if self.__isSummaryKeyObservationKey(key)]
        return self.__observation_keys

    def getCustomPcaKeys(self):
        """ @rtype: list of str """
        return self.__custom_pca_keys.keys()


    def getList(self):
        """ @rtype: list of str """
        self.__keys = self.__keys or self.getAllKeys()
        return self.__keys


    def isObservationKey(self, item):
        """ @rtype: bool """
        if self.__observation_keys is None:
            self.getAllObservationSummaryKeys()

        if self.__block_observation_keys is None:
            self.getAllBlockObservationKeys()

        if self.__gen_data_keys is None:
            self.getAllGenDataKeys()

        return item in self.__observation_keys or item in self.__block_observation_keys or item in self.__gen_data_keys

    def __isSummaryKeyObservationKey(self, key):
        return len(self.ert().ensembleConfig().getNode(key).getObservationKeys()) > 0

    def isSummaryKey(self, key):
        return key in self.__summary_keys

    def isBlockKey(self, key):
        return key in self.__block_observation_keys

    def isGenKWKey(self, key):
        return key in self.__gen_kw_keys

    def isGenDataKey(self, key):
        return key in self.__gen_data_keys

    def isCustomPcaKey(self, key):
        return key in self.__custom_pca_keys

    def getCustomPcaKeyObsKeys(self, key):
        return self.__custom_pca_keys[key]


from ert.enkf.enkf_main import EnKFMain


class DataFetcher(object):
    def __init__(self, ert):
        super(DataFetcher, self).__init__()
        assert isinstance(ert, EnKFMain)
        self.__ert = ert
        self.__supported_keys = None

    def fetchData(self, key, case=None):
        raise NotImplementedError()

    def ert(self):
        return self.__ert

    def fetchSupportedKeys(self):
        raise NotImplementedError()

    def getSupportedKeys(self):
        if self.__supported_keys is None:
            self.__supported_keys = self.fetchSupportedKeys()
        return self.__supported_keys

    def supportsKey(self, key):
        return key in self.getSupportedKeys()





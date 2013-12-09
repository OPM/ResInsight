from ert.enkf.enkf_main import EnKFMain


class DataFetcher(object):
    def __init__(self, ert):
        super(DataFetcher, self).__init__()
        assert isinstance(ert, EnKFMain)
        self.__ert = ert


    def fetchData(self):
        raise NotImplementedError()

    def ert(self):
        return self.__ert





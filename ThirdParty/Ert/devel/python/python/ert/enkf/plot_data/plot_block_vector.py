from ert.util import DoubleVector


class PlotBlockVector(object):
    def __init__(self, realization_number, data):
        """
        @type realization_number: int
        @type data: DoubleVector
        """
        super(PlotBlockVector, self).__init__()

        assert isinstance(data, DoubleVector)

        self.__realization_number = realization_number
        self.__data = data

    def __len__(self):
        """ @rtype: int """
        return len(self.__data)


    def __getitem__(self, index):
        """ @rtype: float """
        assert isinstance(index, int)
        return self.__data[index]


    def __iter__(self):
        """ @rtype: float """
        cur = 0
        while cur < len(self):
            yield self[cur]
            cur += 1


    def getRealizationNumber(self):
        """ @rtype: int """
        return self.__realization_number


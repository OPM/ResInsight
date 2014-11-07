from ert.enkf.plot_data import PlotBlockVector
from ert.util import DoubleVector


class PlotBlockData(object):

    def __init__(self, depth_vector):
        """
        @type depth_vector: DoubleVector
        """
        assert isinstance(depth_vector, DoubleVector)
        self.__depth_vector = depth_vector
        self.__plot_block_vectors = {}


    def __len__(self):
        """ @rtype: int """
        return len(self.__plot_block_vectors)


    def __getitem__(self, index):
        """
        @type index: int
        @rtype: PlotBlockVector
        """
        return self.__plot_block_vectors[index]

    def __iter__(self):
        cur = 0
        keys = sorted(self.__plot_block_vectors.keys())
        while cur < len(keys):
            yield self[keys[cur]]
            cur += 1

    def getDepth(self):
        """ @rtype: DoubleVector """
        return self.__depth_vector


    def addPlotBlockVector(self, vector):
        """
        @type vector: PlotBlockVector
        """
        self.__plot_block_vectors[vector.getRealizationNumber()] = vector

import pandas as pd
from pandas import DataFrame


class DesignMatrixReader(object):

    @staticmethod
    def loadDesignMatrix(filename):
        """@rtype: DataFrame"""
        dm = pd.read_csv(filename, delim_whitespace=True)
        """ @type dm: pd.DataFrame """
        dm = dm.rename(columns={dm.columns[0]:"Realization"})
        dm = dm.set_index(["Realization"])

        return dm

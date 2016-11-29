from __future__ import print_function
import math
from pandas import DataFrame, MultiIndex
import numpy
from ert.enkf import ErtImplType, EnKFMain, EnkfFs, RealizationStateEnum, GenKwConfig
from ert.enkf.plot_data import EnsemblePlotGenData
from ert.util import BoolVector


class ArgLoader(object):

    @staticmethod
    def load(filename , column_names = None):
        rows = 0
        columns = 0
        with open(filename,"r") as fileH:
            for line in fileH.readlines():
                rows += 1
                columns = max(columns , len( line.split()) )

        if not column_names is None:
            if len(column_names) <= columns:
                columns = len(column_names)
            else:
                raise ValueError("To many coloumns in input")

        data = numpy.empty(shape=(rows , columns) , dtype=numpy.float64)
        data.fill( numpy.nan )

        row = 0
        with open(filename) as fileH:
            for line in fileH.readlines():
                tmp = line.split( )
                print(tmp)
                for column in range(columns):
                    data[row][column] = float(tmp[column])
                row += 1


        if column_names is None:
            column_names = []
            for column in range(columns):
                column_names.append( "Column%d" % column )

        data_frame = DataFrame( data = data , columns = column_names)
        return data_frame

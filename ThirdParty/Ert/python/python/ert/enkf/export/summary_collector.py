from pandas import DataFrame, MultiIndex
import numpy
from ert.enkf import ErtImplType, EnKFMain, EnkfFs, RealizationStateEnum
from ert.enkf.key_manager import KeyManager
from ert.enkf.plot_data import EnsemblePlotData
from ert.util import BoolVector


class SummaryCollector(object):

    @staticmethod
    def createActiveList(ert, fs):
        state_map = fs.getStateMap()
        ens_mask = BoolVector(False, ert.getEnsembleSize())
        state_map.selectMatching(ens_mask, RealizationStateEnum.STATE_HAS_DATA)
        active_list = BoolVector.createActiveList(ens_mask)

        return [iens for iens in active_list]

    @staticmethod
    def getAllSummaryKeys(ert):
        """ @rtype: list of str """
        key_manager = KeyManager(ert)
        return key_manager.summaryKeys()

    @staticmethod
    def loadAllSummaryData(ert, case_name, keys=None):
        """
        @type ert: EnKFMain
        @type case_name: str
        @type keys: list of str
        @rtype: DataFrame
        """
        fs = ert.getEnkfFsManager().getFileSystem(case_name)

        time_map = fs.getTimeMap()
        dates = [time_map[index].datetime() for index in range(1, len(time_map))]
        realizations = SummaryCollector.createActiveList(ert, fs)

        summary_keys = SummaryCollector.getAllSummaryKeys(ert)
        if keys is not None:
            summary_keys = [key for key in keys if key in summary_keys] # ignore keys that doesn't exist

        summary_array = numpy.empty(shape=(len(summary_keys), len(realizations) * len(dates)), dtype=numpy.float64)
        summary_array.fill(numpy.nan)

        for key_index, key in enumerate(summary_keys):
            ensemble_config_node = ert.ensembleConfig().getNode(key)
            ensemble_data = EnsemblePlotData(ensemble_config_node, fs)
            summary_row = summary_array[key_index]

            for realization_index, realization_number in enumerate(realizations):
                realization_vector = ensemble_data[realization_number]
                column_index = realization_index * len(dates)

                for index in range(1, len(realization_vector)):
                    if realization_vector.isActive(index):
                        # assert time_map[index] == realization_vector.getTime(index)
                        # assert time_map[index].datetime() == dates[index - 1]
                        value = realization_vector.getValue(index)
                        summary_row[column_index + index - 1] = value


        multi_index = MultiIndex.from_product([realizations, dates], names=["Realization", "Date"])
        summary_data = DataFrame(data=numpy.transpose(summary_array), index=multi_index, columns=summary_keys)
        return summary_data


from pandas import DataFrame
import numpy
from ert.enkf import EnKFMain, EnkfFs, RealizationStateEnum
from ert.enkf.key_manager import KeyManager
from ert.util import BoolVector


class MisfitCollector(object):

    @staticmethod
    def createActiveList(ert, fs):
        state_map = fs.getStateMap()
        ens_mask = BoolVector(False, ert.getEnsembleSize())
        state_map.selectMatching(ens_mask, RealizationStateEnum.STATE_HAS_DATA)
        active_list = BoolVector.createActiveList(ens_mask)

        return [iens for iens in active_list]

    @staticmethod
    def getAllMisfitKeys(ert, sort_keys=True):
        """ @rtype: list of str """
        key_manager = KeyManager(ert)
        return key_manager.misfitKeys(sort_keys=sort_keys)

    @staticmethod
    def loadAllMisfitData(ert, case_name):
        """
        @type ert: EnKFMain
        @type case_name: str
        @rtype: DataFrame
        """
        fs = ert.getEnkfFsManager().getFileSystem(case_name)

        realizations = MisfitCollector.createActiveList(ert, fs)
        misfit_keys = MisfitCollector.getAllMisfitKeys(ert, sort_keys=False)
        misfit_sum_index = len(misfit_keys) - 1

        misfit_array = numpy.empty(shape=(len(misfit_keys), len(realizations)), dtype=numpy.float64)
        misfit_array.fill(numpy.nan)
        misfit_array[misfit_sum_index] = 0.0

        for column_index, obs_vector in enumerate(ert.getObservations()):

            for realization_index, realization_number in enumerate(realizations):
                misfit = obs_vector.getTotalChi2(fs, realization_number)

                misfit_array[column_index][realization_index] = misfit
                misfit_array[misfit_sum_index][realization_index] += misfit

        misfit_data = DataFrame(data=numpy.transpose(misfit_array), index=realizations, columns=misfit_keys)
        misfit_data.index.name = "Realization"

        return misfit_data

import math
from pandas import DataFrame, MultiIndex
import numpy
from ert.enkf import ErtImplType, EnKFMain, EnkfFs, RealizationStateEnum, GenKwConfig
from ert.enkf.key_manager import KeyManager
from ert.enkf.plot_data import EnsemblePlotGenKW
from ert.util import BoolVector


class GenKwCollector(object):

    @staticmethod
    def createActiveList(ert, fs):
        state_map = fs.getStateMap()
        ens_mask = BoolVector(False, ert.getEnsembleSize())
        state_map.selectMatching(ens_mask, RealizationStateEnum.STATE_INITIALIZED | RealizationStateEnum.STATE_HAS_DATA)
        active_list = BoolVector.createActiveList(ens_mask)

        return [iens for iens in active_list]

    @staticmethod
    def getAllGenKwKeys(ert):
        """ @rtype: list of str """
        key_manager = KeyManager(ert)
        return key_manager.genKwKeys()

    @staticmethod
    def loadAllGenKwData(ert, case_name, keys=None):
        """
        @type ert: EnKFMain
        @type case_name: str
        @type keys: list of str
        @rtype: DataFrame
        """
        fs = ert.getEnkfFsManager().getFileSystem(case_name)

        realizations = GenKwCollector.createActiveList(ert, fs)

        gen_kw_keys = GenKwCollector.getAllGenKwKeys(ert)

        if keys is not None:
            gen_kw_keys = [key for key in keys if key in gen_kw_keys] # ignore keys that doesn't exist

        gen_kw_array = numpy.empty(shape=(len(gen_kw_keys), len(realizations)), dtype=numpy.float64)
        gen_kw_array.fill(numpy.nan)

        for column_index, key in enumerate(gen_kw_keys):
            key, keyword = key.split(":")

            use_log_scale = False
            if key.startswith("LOG10_"):
                key = key[6:]
                use_log_scale = True

            ensemble_config_node = ert.ensembleConfig().getNode(key)
            ensemble_data = EnsemblePlotGenKW(ensemble_config_node, fs)
            keyword_index = ensemble_data.getIndexForKeyword(keyword)

            for realization_index, realization_number in enumerate(realizations):
                realization_vector = ensemble_data[realization_number]

                value = realization_vector[keyword_index]

                if use_log_scale:
                    value = math.log10(value)

                gen_kw_array[column_index][realization_index] = value

        gen_kw_data = DataFrame(data=numpy.transpose(gen_kw_array), index=realizations, columns=gen_kw_keys)
        gen_kw_data.index.name = "Realization"

        return gen_kw_data


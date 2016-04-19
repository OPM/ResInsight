from pandas import DataFrame
from ert.enkf import EnKFMain, EnkfFs, EnkfObservationImplementationType


class GenDataObservationCollector(object):

    @staticmethod
    def getAllObservationKeys(ert):
        """
         @type ert: EnKFMain
         @rtype: list of str
        """
        enkf_obs = ert.getObservations()
        observation_keys = enkf_obs.getTypedKeylist(EnkfObservationImplementationType.GEN_OBS)
        return [key for key in observation_keys]

    @staticmethod
    def getObservationKeyForDataKey(ert, data_key, data_report_step):
        """
         @type ert: EnKFMain
         @rtype: str
        """
        observation_key = None

        enkf_obs = ert.getObservations()
        for obs_vector in enkf_obs:
            report_step = obs_vector.activeStep()
            key = obs_vector.getDataKey()

            if key == data_key and report_step == data_report_step:
                observation_key = obs_vector.getObservationKey()

        return observation_key


    @staticmethod
    def loadGenDataObservations(ert, case_name, key):
        """
        @type ert: EnKFMain
        @type case_name: str
        @type key: name of an observation key
        @rtype: DataFrame
        """
        fs = ert.getEnkfFsManager().getFileSystem(case_name)

        available_observation_keys = GenDataObservationCollector.getAllObservationKeys(ert)
        if not key in available_observation_keys:
            raise KeyError("Key '%s' is not a valid observation key")

        columns = [key]
        std_columns = ["STD_%s" % key]

        enkf_obs = ert.getObservations()

        index_set = set()
        obs_vector = enkf_obs[key]
        report_step = obs_vector.activeStep()

        obs_node = obs_vector.getNode(report_step)
        # """ :type: ert.enkf.observations.GenObservation """

        for obs_index in range(len(obs_node)):
            index_set.add(obs_node.getIndex(obs_index))

        index_list = sorted(list(index_set))
        data = DataFrame(index=index_list, columns=columns + std_columns)

        for obs_index, (value, std) in enumerate(obs_node):
            data_index = obs_node.getIndex(obs_index)
            data[key][data_index] = value
            data["STD_%s" % key][data_index] = std

        return data

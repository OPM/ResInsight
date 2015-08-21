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
    def loadGenDataObservations(ert, case_name, keys=None):
        """
        @type ert: EnKFMain
        @type case_name: str
        @type key: list of str
        @rtype: DataFrame
        """
        fs = ert.getEnkfFsManager().getFileSystem(case_name)

        observation_keys = GenDataObservationCollector.getAllObservationKeys(ert)
        if keys is not None:
            observation_keys = [key for key in keys if key in observation_keys] # ignore keys that doesn't exist

        columns = observation_keys
        std_columns = ["STD_%s" % key for key in observation_keys]

        enkf_obs = ert.getObservations()

        max_size = 0
        for key in observation_keys:
            obs_vector = enkf_obs[key]
            report_step = obs_vector.activeStep()
            obs_node = obs_vector.getNode(report_step)
            """ :type: ert.enkf.observations.GenObservation """
            max_size = max(obs_node.getSize(), max_size)

        data = DataFrame(index=range(max_size), columns=columns + std_columns)
        for key in observation_keys:
            obs_vector = enkf_obs[key]
            report_step = obs_vector.activeStep()
            obs_node = obs_vector.getNode(report_step)
            """ :type: ert.enkf.observations.GenObservation """

            for iobs , (value,std) in enumerate(obs_node):
                data_index = obs_node.getDataIndex( iobs )
                data[key][data_index] = value
                data["STD_%s" % key][data_index] = std

        return data

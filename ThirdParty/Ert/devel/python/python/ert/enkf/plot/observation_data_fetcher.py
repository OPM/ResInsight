from ert.enkf import EnkfObservationImplementationType
from ert.enkf.plot import DataFetcher, Sample, SampleListCollection, SampleList


class ObservationDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(ObservationDataFetcher, self).__init__(ert)


    def getObservationKeys(self):
        observations = self.ert().getObservations()
        keys = observations.getTypedKeylist(EnkfObservationImplementationType.SUMMARY_OBS)
        keys = sorted(keys)
        return keys

    def getFirstReportStep(self):
        return self.ert().getObservations().getObservationTime(0).ctime()

    def getLastReportStep(self):
        history_length = self.ert().getHistoryLength()
        return self.ert().getObservations().getObservationTime(history_length - 1).ctime()


    def getObservationsForKey(self, key):
        """ @rtype: SampleList """
        obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()

        if len(obs_keys) == 0:
            return None

        sample_list = SampleList()
        sample_list.group = key
        sample_list.min_x = self.getFirstReportStep()
        sample_list.max_x = self.getLastReportStep()

        for obs_key in obs_keys:
            observations = self.getObservations(obs_key)

            for observation in observations:
                sample_list.addSample(observation)

        return sample_list


    def getObservations(self, key):
        """ @rtype: list of Sample """
        observations = self.ert().getObservations()
        assert observations.hasKey(key)
        observation_data = observations.getObservationsVector(key)
        active_count = observation_data.getActiveCount()

        result = []
        history_length = self.ert().getHistoryLength()
        for index in range(0, history_length):
            if observation_data.isActive(index):
                sample = Sample()
                sample.index = index
                sample.x = observations.getObservationTime(index).ctime()

                #: :type: SummaryObservation
                node = observation_data.getNode(index)

                sample.y = node.getValue()
                sample.std = node.getStandardDeviation()
                sample.group = node.getSummaryKey()
                sample.name = key

                if active_count == 1:
                    sample.single_point = True

                result.append(sample)

        return result

    def getAllObservations(self):
        keys = self.getObservationKeys()

        result = SampleListCollection()

        for key in keys:
            observations = self.getObservations(key)

            for observation in observations:
                if not observation.group in result:
                    sample_list = SampleList()
                    sample_list.group = observation.group
                    sample_list.min_x = self.getFirstReportStep()
                    sample_list.max_x = self.getLastReportStep()

                    result.addSampleList(sample_list)

                result[observation.group].addSample(observation)

        return result

    def fetchData(self):
        return self.getAllObservations()



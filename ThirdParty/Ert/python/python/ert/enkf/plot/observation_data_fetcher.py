from ert.enkf import EnkfObservationImplementationType
from ert.enkf.enums import ErtImplType
from ert.enkf.plot import DataFetcher


class ObservationDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(ObservationDataFetcher, self).__init__(ert)

    def getObservationKeys(self):
        observations = self.ert().getObservations()
        keys = observations.getTypedKeylist(EnkfObservationImplementationType.SUMMARY_OBS)
        keys = sorted(keys)
        return keys

    def fetchSupportedKeys(self):
        """ @rtype: list of str """
        return sorted([key for key in self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)])

    def __getObservationData(self, key, data):
        observations = self.ert().getObservations()
        assert observations.hasKey(key)

        observation_data = observations[key]
        active_count = observation_data.getActiveCount()

        history_length = self.ert().getHistoryLength()
        for index in range(0, history_length):
            if observation_data.isActive(index):
                x_value = int(observations.getObservationTime(index).ctime())
                data["x"].append(x_value)

                #: :type: SummaryObservation
                node = observation_data.getNode(index)

                y_value = node.getValue()
                std = node.getStandardDeviation()
                data["y"].append(float(y_value))
                data["std"].append(float(std))

                if data["min_x"] is None or data["min_x"] > x_value:
                    data["min_x"] = x_value

                if data["max_x"] is None or data["max_x"] < x_value:
                    data["max_x"] = x_value


                adjusted_y = self.adjustY(y_value, std)

                if data["min_y"] is None or data["min_y"] > adjusted_y:
                    data["min_y"] = adjusted_y

                if data["max_y"] is None or data["max_y"] < y_value + std:
                    data["max_y"] = y_value + std

                if active_count == 1:
                    data["continuous"] = False

    @staticmethod
    def adjustY(y, std):
        if y >= 0:
            return max(0, y - std)

        return y - std


    def fetchData(self, key, case=None):
        obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()
        history_length = self.ert().getHistoryLength()

        data = {"continuous": True,
                "x": None,
                "y": None,
                "std": None,
                "min_y": None,
                "max_y": None,
                "min_x": None,
                "max_x": None}

        if len(obs_keys) == 0:
            return data

        data["x"] = []
        data["y"] = []
        data["std"] = []

        for obs_key in obs_keys:
            self.__getObservationData(obs_key, data)

        return data


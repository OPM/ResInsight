# Copyright (C) 2014  Statoil ASA, Norway.
#
# The file 'observation_gen_data_fetcher.py' is part of ERT - Ensemble based Reservoir Tool.
#
# ERT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ERT is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.
#
# See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
# for more details.
from ert.enkf.enums.ert_impl_type_enum import ErtImplType
from ert.enkf.plot import DataFetcher


class ObservationGenDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(ObservationGenDataFetcher, self).__init__(ert)

    def fetchSupportedKeys(self):
        gen_data_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_DATA)
        gen_data_list = []
        for key in gen_data_keys:
            obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()
            for obs_key in obs_keys:
                obs_vector = self.ert().getObservations()[obs_key]
                for report_step in obs_vector.getStepList():
                    gen_data_list.append("%s@%d" % (key, report_step))

        return gen_data_list

    def __getObservationData(self, key, report_step):
        data = {"continuous": True,
                "x": [],
                "y": [],
                "std": [],
                "min_y": None,
                "max_y": None,
                "min_x": None,
                "max_x": None}

        observations = self.ert().getObservations()
        assert observations.hasKey(key)

        gen_obs = observations[key].getNode(report_step)

        size = gen_obs.getSize()

        data["min_x"] = 0
        data["max_x"] = size - 1
        for index in range(0, size):
            std = gen_obs.getStandardDeviation(index)
            data["std"].append(std)
            y_value = gen_obs.getValue(index)
            data["y"].append(y_value)

            adjusted_y = self.adjustY(y_value, std)

            if data["min_y"] is None or data["min_y"] > adjusted_y:
                 data["min_y"] = adjusted_y

            if data["max_y"] is None or data["max_y"] < y_value + std:
                 data["max_y"] = y_value + std

            obs_index = gen_obs.getIndex(index)
            data["x"].append(obs_index)

        return data


    def getObsKeyForKey(self, key, key_report_step):
        obs_keys = self.ert().ensembleConfig().getNode(key).getObservationKeys()
        for obs_key in obs_keys:
            obs_vector = self.ert().getObservations()[obs_key]
            for report_step in obs_vector.getStepList():
                if report_step == key_report_step:
                    return obs_key

        raise UserWarning("Observation key for key '%s' not found!" % key)


    def getAllObsKeysForKey(self, key):
        key, report_step = key.split("@")
        return self.ert().ensembleConfig().getNode(key).getObservationKeys()


    def hasData(self, key):
        """ @rtype: bool """
        key, report_step = key.split("@")
        observations = self.ert().getObservations()
        obs_key = self.getObsKeyForKey(key, int(report_step))
        if not observations.hasKey(obs_key):
            return False

        return observations[obs_key].getActiveCount() > 0

    def fetchData(self, key, case=None):
        key, report_step = key.split("@")

        key_report_step = int(report_step)
        obs_key = self.getObsKeyForKey(key, key_report_step)

        return self.__getObservationData(obs_key, key_report_step)


    @staticmethod
    def adjustY(y, std):
        if y >= 0:
            return max(0, y - std)

        return y - std

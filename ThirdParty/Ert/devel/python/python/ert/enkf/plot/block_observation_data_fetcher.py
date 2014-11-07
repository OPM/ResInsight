# Copyright (C) 2013  Statoil ASA, Norway.
#
# The file 'block_observation_data_fetcher.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf import EnkfObservationImplementationType
from ert.enkf.observations import BlockObservation
from ert.enkf.plot import DataFetcher


class BlockObservationDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(BlockObservationDataFetcher, self).__init__(ert)
        self.__selected_report_step_index = None

    def setSelectedReportStepIndex(self, index):
        self.__selected_report_step_index = index

    def fetchSupportedKeys(self):
        observations = self.ert().getObservations()
        string_list = observations.getTypedKeylist(EnkfObservationImplementationType.BLOCK_OBS)
        return [key for key in string_list]

    def __fetchObservationData(self, block_observation):
        assert isinstance(block_observation, BlockObservation)

        data = {
            "continuous": False,
            "x": [],
            "y": [],
            "std": [],
            "min_y": None,
            "max_y": None,
            "min_x": None,
            "max_x": None
        }

        for index in block_observation:
            std = block_observation.getStd(index)
            y = block_observation.getDepth(index)
            x = block_observation.getValue(index)

            data["std"].append(std)
            data["y"].append(y)
            data["x"].append(x)

            adjusted_x = self.adjustX(x, std)

            if data["min_x"] is None or data["min_x"] > adjusted_x:
                data["min_x"] = adjusted_x

            if data["max_x"] is None or data["max_x"] < x + std:
                data["max_x"] = x + std


            if data["min_y"] is None or data["min_y"] > y:
                data["min_y"] = y

            if data["max_y"] is None or data["max_y"] < y:
                data["max_y"] = y

        return data

    @staticmethod
    def adjustX(x, std):
        if x >= 0:
            return max(0, x - std)

        return x - std

    def hasData(self, key):
        """ @rtype: bool """
        observations = self.ert().getObservations()
        if not observations.hasKey(key):
            return False

        return observations[key].getActiveCount() > 0

    def fetchData(self, key, case=None):
        observations = self.ert().getObservations()
        assert observations.hasKey(key)

        observation_vector = observations[key]

        report_step_data = []
        for report_step in observation_vector:
            block_observation = observation_vector.getNode(report_step)
            data = self.__fetchObservationData(block_observation)
            data["report_step"] = report_step
            report_step_data.append(data)


        if self.__selected_report_step_index is not None:
            return report_step_data[self.__selected_report_step_index]
        else:
            return report_step_data


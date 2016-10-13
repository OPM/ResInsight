# Copyright (C) 2014  Statoil ASA, Norway.
#
# The file 'ensemble_gen_data_fetcher.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf.plot_data import EnsemblePlotGenData
from ert.enkf.plot import DataFetcher


class EnsembleGenDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(EnsembleGenDataFetcher, self).__init__(ert)

    def fetchSupportedKeys(self):
        gen_data_list = []

        return gen_data_list

    def getEnsembleConfigNode(self, key):
        """ @rtype: EnsembleConfig """
        ensemble_config = self.ert().ensembleConfig()
        assert key in ensemble_config
        return ensemble_config.getNode(key)

    def fetchData(self, key, case=None):
        key, report_step = key.split("@")
        report_step = int(report_step)

        ensemble_config_node = self.getEnsembleConfigNode(key)
        enkf_fs = self.ert().getEnkfFsManager().getFileSystem(case)
        ensemble_plot_gen_data = EnsemblePlotGenData(ensemble_config_node, enkf_fs, report_step)

        data = {"x": [],
                "y": [],
                "min_y_values": [value for value in ensemble_plot_gen_data.getMinValues()],
                "max_y_values": [value for value in ensemble_plot_gen_data.getMaxValues()],
                "min_y": None,
                "max_y": None,
                "min_x": 0,
                "max_x": None}

        data["x"] = [index for index in range(len(data["min_y_values"]))]
        data["max_x"] = len(data["min_y_values"]) - 1

        for vector in ensemble_plot_gen_data:
            y = []
            data["y"].append(y)

            for value in vector:
                y.append(value)

                if data["min_y"] is None or data["min_y"] > value:
                    data["min_y"] = value

                if data["max_y"] is None or data["max_y"] < value:
                    data["max_y"] = value



        return data


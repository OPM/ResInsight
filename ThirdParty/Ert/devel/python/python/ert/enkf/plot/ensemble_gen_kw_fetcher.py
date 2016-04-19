# Copyright (C) 2014  Statoil ASA, Norway.
#
# The file 'ensemble_gen_kw_fetcher.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.enkf.config.gen_kw_config import GenKwConfig
from ert.enkf.enums.ert_impl_type_enum import ErtImplType
from ert.enkf.plot import DataFetcher
from ert.enkf.plot_data import EnsemblePlotGenKW


class EnsembleGenKWFetcher(DataFetcher):
    def __init__(self, ert):
        super(EnsembleGenKWFetcher, self).__init__(ert)

    def fetchSupportedKeys(self):
        gen_kw_keys = self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.GEN_KW)
        gen_kw_list = []
        for key in gen_kw_keys:
            enkf_config_node = self.ert().ensembleConfig().getNode(key)
            model_config = enkf_config_node.getModelConfig()
            assert isinstance(model_config, GenKwConfig)

            model_keys = model_config.getKeyWords()
            for key_word in model_keys:
                gen_kw_list.append("%s:%s" % (key, key_word))

        return gen_kw_list


    def getEnsembleConfigNode(self, key):
        """ @rtype: EnsembleConfig """
        ensemble_config = self.ert().ensembleConfig()
        assert key in ensemble_config
        return ensemble_config.getNode(key)


    def fetchData(self, key, case=None):
        key, keyword = key.split(":")
        ensemble_config_node = self.getEnsembleConfigNode(key)
        enkf_fs = self.ert().getEnkfFsManager().getFileSystem(case)
        ensemble_plot_gen_kw = EnsemblePlotGenKW(ensemble_config_node, enkf_fs)
        keyword_index = ensemble_plot_gen_kw.getIndexForKeyword(keyword)

        use_log_scale = ensemble_plot_gen_kw.shouldUseLogScale(keyword_index)

        data = {"x": [0],
                "y": [],
                "min_y": None,
                "max_y": None,
                "min_x": 0,
                "max_x": 0,
                "use_log_scale": use_log_scale}

        for vector in ensemble_plot_gen_kw:
            y = []
            data["y"].append(y)

            if len(vector) > keyword_index:
                value = vector.getValue(keyword_index)
                y.append(value)

                if data["min_y"] is None or data["min_y"] > value:
                    data["min_y"] = value

                if data["max_y"] is None or data["max_y"] < value:
                    data["max_y"] = value

        return data


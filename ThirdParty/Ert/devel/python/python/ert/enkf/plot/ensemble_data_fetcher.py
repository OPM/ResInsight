from ert.enkf import EnsembleConfig
from ert.enkf.plot_data import EnsemblePlotData
from ert.enkf.enums import ErtImplType
from ert.enkf.plot.data_fetcher import DataFetcher


class EnsembleDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(EnsembleDataFetcher, self).__init__(ert)

    def fetchSupportedKeys(self):
        """ @rtype: list of str """
        return [key for key in self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)]


    def getEnsembleConfigNode(self, key):
        """ @rtype: EnsembleConfig """
        ensemble_config = self.ert().ensembleConfig()
        assert key in ensemble_config
        return ensemble_config.getNode(key)


    def fetchData(self, key, case=None):
        ensemble_config_node = self.getEnsembleConfigNode(key)
        enkf_fs = self.ert().getEnkfFsManager().getFileSystem(case)
        ensemble_plot_data = EnsemblePlotData(ensemble_config_node, enkf_fs)

        data = {
            "x": [],
            "y": [],
            "min_y_values": [],
            "max_y_values": [],
            "min_y": None,
            "max_y": None,
            "min_x": None,
            "max_x": None
        }

        time_map = enkf_fs.getTimeMap()

        for index in range(1, len(time_map)):
            data["x"].append(time_map[index].ctime())
            data["min_y_values"].append(None)
            data["max_y_values"].append(None)

        data["min_x"] = data["x"][0]
        data["max_x"] = data["x"][len(data["x"]) - 1]


        for vector in ensemble_plot_data:
            y = []
            data["y"].append(y)

            # skip index 0 (not a valid simulation value...)
            for index in range(len(vector) - 1):
                if vector.isActive(index + 1):
                    y_value = vector.getValue(index + 1)
                    y.append(y_value)

                    if data["min_y"] is None or data["min_y"] > y_value:
                        data["min_y"] = y_value

                    if data["max_y"] is None or data["max_y"] < y_value:
                        data["max_y"] = y_value


                    if data["min_y_values"][index] is None or data["min_y_values"][index] > y_value:
                        data["min_y_values"][index] = y_value

                    if data["max_y_values"][index] is None or data["max_y_values"][index] < y_value:
                        data["max_y_values"][index] = y_value

        return data

from ert.enkf import EnsConfig
from ert.enkf.ensemble_data import EnsemblePlotData, EnsemblePlotDataVector
from ert.enkf.enums import ErtImplType
from ert.enkf.plot import SimpleSample, SampleListCollection, SampleList
from ert.enkf.plot.data.sample_statistics import SampleStatistics
from ert.enkf.plot.data_fetcher import DataFetcher


class EnsembleDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(EnsembleDataFetcher, self).__init__(ert)
        self.report_times = {}


    def fetchData(self):
        """ @rtype: SampleListCollection """
        return self.getEnsembleData()

    def getSummaryKeys(self):
        """ @rtype: StringList """
        return self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)


    def getEnsembleConfigNode(self, key):
        """ @rtype: EnsConfig """
        ensemble_config = self.ert().ensembleConfig()
        assert ensemble_config.hasKey(key)
        return ensemble_config.getNode(key)

    def getRealizationData(self, key, ensemble_plot_data_vector):
        """ @rtype: SampleList """
        assert isinstance(ensemble_plot_data_vector, EnsemblePlotDataVector)

        samples = SampleList()
        samples.group = key
        active_count = 0
        for index in range(len(ensemble_plot_data_vector)):
            if ensemble_plot_data_vector.isActive(index):
                sample = SimpleSample()
                sample.y = ensemble_plot_data_vector.getValue(index)
                sample.x = ensemble_plot_data_vector.getTime(index).ctime()

                samples.addSample(sample)
                active_count += 1

        samples.min_x = samples.statistics.min_x
        samples.max_x = samples.statistics.max_x

        return samples



    def getEnsembleDataForKey(self, key):
        """ @rtype: list of SampleList """
        ensemble_config_node = self.getEnsembleConfigNode(key)
        enkf_fs = self.ert().getEnkfFsManager().getFileSystem()
        enkf_plot_data = EnsemblePlotData(ensemble_config_node, enkf_fs)

        result = []
        for index in range(len(enkf_plot_data)):
            result.append(self.getRealizationData(key, enkf_plot_data[index]))


        return result


    def getEnsembleData(self):
        keys = self.getSummaryKeys()

        result = {}
        for key in keys:
            result[key] = self.getEnsembleDataForKey(key)

        return result


    def getEnsembleDataForKeyAndCase(self, key, case):
        """ @rtype: list of SampleList """
        ensemble_config_node = self.getEnsembleConfigNode(key)
        enkf_fs = self.ert().getEnkfFsManager().mountAlternativeFileSystem(case, True, False)
        ensemble_plot_data = EnsemblePlotData(ensemble_config_node, enkf_fs)

        statistics = []
        result = []
        for index in range(len(ensemble_plot_data)):
            sample_list = self.getRealizationData(key, ensemble_plot_data[index])
            result.append(sample_list)

            for index in range(len(sample_list.samples)):
                if index == len(statistics):
                    statistics.append(SampleStatistics())
                statistics[index].addSample(sample_list.samples[index])


        return result, statistics











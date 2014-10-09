from ert.enkf.plot import EnsembleDataFetcher, ObservationDataFetcher, RefcaseDataFetcher, BlockObservationDataFetcher, EnsembleGenKWFetcher, EnsembleGenDataFetcher, ObservationGenDataFetcher
from ert.enkf.plot import EnsembleBlockDataFetcher, PcaDataFetcher
from ert_gui.models.connectors.plot import DataTypeKeysModel
from ert_gui.tools.plot.data import PlotData, ObservationPlotData, EnsemblePlotData, RefcasePlotData, HistogramPlotDataFactory, ReportStepLessHistogramPlotDataFactory
from ert_gui.models import ErtConnector


class PlotDataFetcher(ErtConnector):

    def getPlotDataForKeyAndCases(self, key, cases):
        observation_data_fetcher = ObservationDataFetcher(self.ert())
        block_observation_data_fetcher = BlockObservationDataFetcher(self.ert())
        gen_kw_fetcher = EnsembleGenKWFetcher(self.ert())
        gen_data_fetcher = EnsembleGenDataFetcher(self.ert())

        if self.isBlockObservationKey(key):
            return self.fetchBlockObservationData(block_observation_data_fetcher, key, cases)

        elif self.isSummaryKey(key):
            return self.fetchSummaryData(observation_data_fetcher, key, cases)

        elif self.isGenKWKey(key):
            return self.fetchGenKWData(gen_kw_fetcher, key, cases)

        elif self.isGenDataKey(key):
            return self.fetchGenData(gen_data_fetcher, key, cases)

        elif self.isPcaDataKey(key):
            plot_data = PlotData(key)
            pca_plot_data = self.fetchPcaData(key, cases)
            plot_data.setUserData("PCA", pca_plot_data)
            return plot_data

        else:
            raise NotImplementedError("Key %s not supported." % key)


    def isSummaryKey(self, key):
        ensemble_data_fetcher = ObservationDataFetcher(self.ert())
        return ensemble_data_fetcher.supportsKey(key)


    def isBlockObservationKey(self, key):
        block_observation_data_fetcher = BlockObservationDataFetcher(self.ert())
        return block_observation_data_fetcher.supportsKey(key)


    def isGenKWKey(self, key):
        gen_kw_fetcher = EnsembleGenKWFetcher(self.ert())
        return gen_kw_fetcher.supportsKey(key)


    def isGenDataKey(self, key):
        obs_gen_data_fetcher = ObservationGenDataFetcher(self.ert())
        return obs_gen_data_fetcher.supportsKey(key)


    def isPcaDataKey(self, key):
        pca_data_fetcher = PcaDataFetcher(self.ert())
        return pca_data_fetcher.supportsKey(key) or DataTypeKeysModel().isCustomPcaKey(key)

    def isCustomPcaDataKey(self, key):
        return DataTypeKeysModel().isCustomPcaKey(key)

    def dataTypeKeySupportsReportSteps(self, key):
        return self.isSummaryKey(key)

    def fetchGenData(self, gen_data_fetcher, key, cases):
        plot_data = PlotData(key)

        gen_data_observation_fetcher = ObservationGenDataFetcher(self.ert())

        if gen_data_observation_fetcher.hasData(key):
            self.addObservationData(plot_data, key, gen_data_observation_fetcher)
            self.addEnsembleData(plot_data, key, cases, gen_data_fetcher)
            self.addPcaData(plot_data, key, cases)

        return plot_data


    def fetchGenKWData(self, gen_kw_fetcher, key, cases):
        plot_data = PlotData(key)

        histogram_factory = ReportStepLessHistogramPlotDataFactory(key)
        self.addEnsembleData(plot_data, key, cases, gen_kw_fetcher, histogram_factory)
        plot_data.setHistogramFactory(histogram_factory)

        return plot_data


    def fetchBlockObservationData(self, block_observation_data_fetcher, key, cases):
        plot_data = PlotData(key)

        plot_data.setUnitY(self.ert().eclConfig().getDepthUnit())
        plot_data.setUnitX(self.ert().eclConfig().getPressureUnit())

        if block_observation_data_fetcher.hasData(key):
            block_observation_data_fetcher.setSelectedReportStepIndex(0)
            self.addObservationData(plot_data, key, block_observation_data_fetcher)

            ensemble_block_data_fetcher = EnsembleBlockDataFetcher(self.ert())
            ensemble_block_data_fetcher.setSelectedReportStepIndex(0)
            self.addEnsembleData(plot_data, key, cases, ensemble_block_data_fetcher)

            self.addPcaData(plot_data, key, cases)

        return plot_data


    def fetchSummaryData(self, observation_data_fetcher, key, cases):
        plot_data = PlotData(key)

        histogram_factory = HistogramPlotDataFactory(key)
        refcase_fetcher = RefcaseDataFetcher(self.ert())

        self.addObservationData(plot_data, key, observation_data_fetcher, histogram_factory)

        self.addRefcaseData(plot_data, key, refcase_fetcher, histogram_factory)

        self.addEnsembleData(plot_data, key, cases, EnsembleDataFetcher(self.ert()), histogram_factory)

        self.addPcaData(plot_data, key, cases)

        if refcase_fetcher.hasRefcase():
            unit = refcase_fetcher.getRefCase().unit(key)
            if unit != "":
                plot_data.setUnitY(unit)

        plot_data.setHistogramFactory(histogram_factory)

        return plot_data


    def addEnsembleData(self, plot_data, key, cases, fetcher, histogram_factory=None):
        for case in cases:
            ensemble_data = fetcher.fetchData(key, case)

            if "use_log_scale" in ensemble_data:
                plot_data.setShouldUseLogScale(ensemble_data["use_log_scale"])

            if len(ensemble_data) > 0:
                ensemble_plot_data = EnsemblePlotData(key, case)

                if "min_y_values" in ensemble_data:
                    min_values = ensemble_data["min_y_values"]
                elif "min_x_values" in ensemble_data:
                    min_values = ensemble_data["min_x_values"]
                else:
                    min_values = []


                if "max_y_values" in ensemble_data:
                    max_values = ensemble_data["max_y_values"]
                elif "max_x_values" in ensemble_data:
                    max_values = ensemble_data["max_x_values"]
                else:
                    max_values = []

                ensemble_plot_data.setEnsembleData(ensemble_data["x"], ensemble_data["y"], min_values, max_values)
                ensemble_plot_data.updateBoundaries(ensemble_data["min_x"], ensemble_data["max_x"], ensemble_data["min_y"], ensemble_data["max_y"])
                plot_data.addEnsembleData(ensemble_plot_data)

                if histogram_factory is not None:
                    histogram_factory.addEnsembleData(case, ensemble_data["x"], ensemble_data["y"], ensemble_data["min_y"], ensemble_data["max_y"])


    def addObservationData(self, plot_data, key, fetcher, histogram_factory=None):
        observation_data = fetcher.fetchData(key)

        observation_plot_data = ObservationPlotData(key)
        observation_plot_data.setObservationData(observation_data["x"], observation_data["y"], observation_data["std"], observation_data["continuous"])
        observation_plot_data.updateBoundaries(observation_data["min_x"], observation_data["max_x"], observation_data["min_y"], observation_data["max_y"])

        plot_data.setObservationData(observation_plot_data)

        if histogram_factory is not None:
            histogram_factory.setObservations(observation_data["x"], observation_data["y"], observation_data["std"], observation_data["min_y"], observation_data["max_y"])


    def addRefcaseData(self, plot_data, key, fetcher, histogram_factory=None):
        refcase_data = fetcher.fetchData(key)
        refcase_plot_data = RefcasePlotData(key)
        refcase_plot_data.setRefcaseData(refcase_data["x"], refcase_data["y"])
        refcase_plot_data.updateBoundaries(refcase_data["min_x"], refcase_data["max_x"], refcase_data["min_y"], refcase_data["max_y"])
        plot_data.setRefcaseData(refcase_plot_data)

        if histogram_factory is not None:
            histogram_factory.setRefcase(refcase_data["x"], refcase_data["y"], refcase_data["min_y"], refcase_data["max_y"])


    def addPcaData(self, plot_data, key, cases):
        if plot_data.hasObservationData() and plot_data.hasEnsembleData():
            plot_data.setUserData("PCA", self.fetchPcaData(key, cases))
        else:
            plot_data.setUserData("PCA", PlotData("No ensemble data available for %s" % key))


    def fetchPcaData(self, key, cases):
        """ @rtype: PlotData """
        if key.startswith("PCA:"):
            pca_name = key
        else:
            pca_name ="PCA:%s" % key

        pca_data_fetcher = PcaDataFetcher(self.ert())
        pca_plot_data = PlotData(pca_name)

        if DataTypeKeysModel().isCustomPcaKey(key):
            obs_keys = DataTypeKeysModel().getCustomPcaKeyObsKeys(key)
        else:
            obs_keys = pca_data_fetcher.getObsKeys(key)

        for case in cases:

            pca_data = pca_data_fetcher.fetchData(obs_keys, case)

            if pca_data["x"] is not None:

                if not pca_plot_data.hasObservationData():
                    pca_observation_plot_data = ObservationPlotData(pca_name)
                    pca_observation_plot_data.setObservationData(pca_data["x"], pca_data["obs_y"], [0.0 for x in pca_data["x"]], False)
                    pca_observation_plot_data.updateBoundaries(pca_data["min_x"], pca_data["max_x"], pca_data["min_y"], pca_data["max_y"])
                    pca_plot_data.setObservationData(pca_observation_plot_data)

                pca_ensemble_plot_data = EnsemblePlotData(key, case)
                pca_ensemble_plot_data.setEnsembleData(pca_data["x"], pca_data["y"], [], [])
                pca_ensemble_plot_data.updateBoundaries(pca_data["min_x"], pca_data["max_x"], pca_data["min_y"], pca_data["max_y"])
                pca_plot_data.addEnsembleData(pca_ensemble_plot_data)

        return pca_plot_data



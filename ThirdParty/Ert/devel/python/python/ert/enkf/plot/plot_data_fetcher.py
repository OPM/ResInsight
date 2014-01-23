import time
from ert.enkf.plot import DataFetcher, PlotData, SampleListCollection, ObservationDataFetcher, RefcaseDataFetcher
from ert.enkf.plot.ensemble_data_fetcher import EnsembleDataFetcher


class PlotDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(PlotDataFetcher, self).__init__(ert)


    def fetchData(self):
        observations = ObservationDataFetcher(self.ert()).fetchData()
        refcase = RefcaseDataFetcher(self.ert()).fetchData()
        ensemble_plot_data = EnsembleDataFetcher(self.ert()).fetchData()

        result = {}
        result["keys"] = []
        result["observation_keys"] = []
        """ @type: dict of (str, PlotData) """

        for key in refcase.sample_lists_keys:
            sample_list = refcase[key]
            """ @type: SampleList """

            if not key in result:
                result[key] = PlotData()
                result[key].name = key
                result["keys"].append(key)

            result[key].setRefcase(sample_list)


        sorted_keys = sorted(ensemble_plot_data.keys())
        for key in sorted_keys:
            if not key in result:
                result[key] = PlotData()
                result[key].name = key
                result["keys"].append(key)

            result[key].setEnsemble(ensemble_plot_data[key])


        for key in observations.sample_lists_keys:
            sample_list = observations[key]
            """ @type: SampleList """

            if not key in result:
                result[key] = PlotData()
                result[key].name = key
                result["keys"].append(key)

            result["observation_keys"].append(key)
            result[key].setObservations(sample_list)


        result["keys"] = sorted(result["keys"])
        result["observation_keys"] = sorted(result["observation_keys"])

        return result

    def fetchDataForKey(self, key):
        """ @rtype: PlotData """
        plot_data = PlotData()
        plot_data.name = key

        observations = ObservationDataFetcher(self.ert()).getObservationsForKey(key)
        refcase = RefcaseDataFetcher(self.ert()).getRefcaseDataForKey(key)
        ensemble_plot_data = EnsembleDataFetcher(self.ert()).getEnsembleDataForKey(key)

        if not observations is None:
            plot_data.setObservations(observations)

        if not refcase is None:
            plot_data.setRefcase(refcase)

        if len(ensemble_plot_data) > 0:
            plot_data.setEnsemble(ensemble_plot_data)

        return plot_data

    def fetchDataForKeyAndCases(self, key, cases):
        plot_data = PlotData()
        plot_data.name = key

        observations = ObservationDataFetcher(self.ert()).getObservationsForKey(key)
        refcase = RefcaseDataFetcher(self.ert()).getRefcaseDataForKey(key)

        if not observations is None:
            plot_data.setObservations(observations)

        if not refcase is None:
            plot_data.setRefcase(refcase)

        for case in cases:
            ensemble_plot_data, ensemble_statistics = EnsembleDataFetcher(self.ert()).getEnsembleDataForKeyAndCase(key, case)
            if len(ensemble_plot_data) > 0:
                plot_data.addEnsemble(case, ensemble_plot_data, ensemble_statistics)

        return plot_data





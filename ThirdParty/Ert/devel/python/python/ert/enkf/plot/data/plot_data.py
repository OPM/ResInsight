from ert.enkf.plot.data import DictProperty, SampleList


class PlotData(dict):
    observations = DictProperty("observations")
    refcase = DictProperty("refcase")
    ensemble = DictProperty("ensemble")
    ensemble_names = DictProperty("ensemble_names")
    ensemble_statistics = DictProperty("ensemble_statistics")
    name = DictProperty("name")

    min_x = DictProperty("min_x")
    max_x = DictProperty("max_x")
    min_y = DictProperty("min_y")
    max_y = DictProperty("max_y")

    def __init__(self):
        super(PlotData, self).__init__()

        self.observations = None
        self.refcase = None
        self.ensemble = None
        self.ensemble_names = []
        self.ensemble_statistics = []
        self.name = None

        self.min_x = None
        self.max_x = None
        self.min_y = None
        self.max_y = None


    def setRefcase(self, refcase):
        assert isinstance(refcase, SampleList)

        self.refcase = refcase

        if self.min_x is None:
            self.min_x = refcase.min_x
        else:
            self.min_x = min(self.min_x, refcase.min_x)

        if self.max_x is None:
            self.max_x = refcase.max_x
        else:
            self.max_x = max(self.max_x, refcase.max_x)

        if self.min_y is None:
            self.min_y = refcase.statistics.min_y
        else:
            self.min_y = min(self.min_y, refcase.statistics.min_y)

        if self.max_y is None:
            self.max_y = refcase.statistics.max_y
        else:
            self.max_y = max(self.max_y, refcase.statistics.max_y)


    def setObservations(self, observations):
        assert isinstance(observations, SampleList)

        self.observations = observations

        if self.min_x is None:
            self.min_x = observations.min_x
        else:
            self.min_x = min(self.min_x, observations.min_x)

        if self.max_x is None:
            self.max_x = observations.max_x
        else:
            self.max_x = max(self.max_x, observations.max_x)

        if self.min_y is None:
            self.min_y = self.adjustMinValue(observations.statistics.min_y, observations.statistics.min_y_with_std)
        else:
            mv = self.adjustMinValue(observations.statistics.min_y, observations.statistics.min_y_with_std)
            self.min_y = min(self.min_y, mv)

        if self.max_y is None:
            self.max_y = observations.statistics.max_y_with_std
        else:
            self.max_y = max(self.max_y, observations.statistics.max_y_with_std)


    def adjustMinValue(self, value, value_with_std):
        if value >= 0:
            return max(0, value_with_std)

        return value_with_std



    # def setEnsemble(self, ensemble):
    #     self.ensemble = ensemble
    #
    #     for realization in ensemble:
    #         if self.min_x is None:
    #             self.min_x = realization.min_x
    #         else:
    #             self.min_x = min(self.min_x, realization.min_x)
    #
    #         if self.max_x is None:
    #             self.max_x = realization.max_x
    #         else:
    #             self.max_x = max(self.max_x, realization.max_x)
    #
    #         if self.min_y is None:
    #             self.min_y = realization.statistics.min_y
    #         else:
    #             self.min_y = min(self.min_y, realization.statistics.min_y)
    #
    #         if self.max_y is None:
    #             self.max_y = realization.statistics.max_y
    #         else:
    #             self.max_y = max(self.max_y, realization.statistics.max_y)

    def addEnsemble(self, case, ensemble, ensemble_statistics):
        if self.ensemble is None:
            self.ensemble = []

        self.ensemble.append(ensemble)
        self.ensemble_names.append(case)
        self.ensemble_statistics.append(ensemble_statistics)

        for realization in ensemble:
            if self.min_x is None:
                self.min_x = realization.min_x
            else:
                self.min_x = min(self.min_x, realization.min_x)

            if self.max_x is None:
                self.max_x = realization.max_x
            else:
                self.max_x = max(self.max_x, realization.max_x)

            if self.min_y is None:
                self.min_y = realization.statistics.min_y
            else:
                self.min_y = min(self.min_y, realization.statistics.min_y)

            if self.max_y is None:
                self.max_y = realization.statistics.max_y
            else:
                self.max_y = max(self.max_y, realization.statistics.max_y)



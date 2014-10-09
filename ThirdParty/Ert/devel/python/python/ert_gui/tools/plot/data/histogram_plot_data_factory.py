from ert_gui.tools.plot.data import HistogramPlotData


class HistogramPlotDataFactory(object):
    def __init__(self, name):
        super(HistogramPlotDataFactory, self).__init__()
        self.__name = name

        self.__observations = None
        self.__refcase = None
        self.__case_names = []
        self.__ensemble_data = {}

    def setObservations(self, x_values, y_values, std_values, min_y, max_y):
        if x_values is not None:
            self.__observations = {"x_values": x_values,
                                   "y_values": y_values,
                                   "std_values": std_values,
                                   "min_y": min_y,
                                   "max_y": max_y,
                                   "reverse_lookup": {}}

            rl = self.__observations["reverse_lookup"]

            for index in range(len(x_values)):
                x = x_values[index]
                rl[x] = index


    def hasObservationSample(self, x_value):
        return x_value in self.__observations["reverse_lookup"]


    def setRefcase(self, x_values, y_values, min_y, max_y):
        if x_values is not None and y_values is not None:
            self.__refcase = {"x_values": x_values,
                              "y_values": y_values,
                              "min_y": min_y,
                              "max_y": max_y,
                              "reverse_lookup": {}}

            rl = self.__refcase["reverse_lookup"]

            for index in range(len(x_values)):
                x = x_values[index]
                rl[x] = index


    def hasRefcaseSample(self, x_value):
        return x_value in self.__refcase["reverse_lookup"]

    def addEnsembleData(self, case_name, x_values, y_values, min_y, max_y):
        self.__case_names.append(case_name)

        ensemble_data = {"x_values": x_values,
                         "y_values": y_values,
                         "min_y": min_y,
                         "max_y": max_y,
                         "reverse_lookup": {}}

        rl = ensemble_data["reverse_lookup"]

        for index in range(len(x_values)):
            x = x_values[index]
            rl[x] = index

        self.__ensemble_data[case_name] = ensemble_data

    def hasEnsembleSample(self, case_name, x_value):
        ensemble_data = self.__ensemble_data[case_name]
        return x_value in ensemble_data["reverse_lookup"]

    def hasEnsembleData(self):
        return len(self.__case_names) > 0

    def hasObservations(self):
        """ @rtype: bool """
        return self.__observations is not None

    def hasRefcaseData(self):
        """ @rtype: bool """
        return self.__refcase is not None

    def hasRefcase(self):
        """ @rtype: bool """
        return self.__refcase is not None


    def observationIndex(self, x_value):
        return self.__observations["reverse_lookup"][x_value]

    def refcaseIndex(self, x_value):
        return self.__refcase["reverse_lookup"][x_value]

    def ensembleIndex(self, case_name, x_value):
        ensemble = self.__ensemble_data[case_name]
        return ensemble["reverse_lookup"][x_value]

    def ensembleData(self, case_name):
        """ @rtype: EnsemblePlotData """
        return self.__ensemble_data[case_name]

    def getEnsembleSamples(self, case_name, x_value):
        """ @rtype: list of float """
        index = self.ensembleIndex(case_name, x_value)
        ensemble = self.ensembleData(case_name)
        result = []
        for realization in ensemble["y_values"]:
            if len(realization) > 0 and index < len(realization):
                result.append(realization[index])

        return result


    def getHistogramData(self, x_value):
        """ @rtype: HistogramPlotData """
        histogram_data = HistogramPlotData(self.__name, x_value)

        if self.hasObservations():
            if self.hasObservationSample(x_value):
                index = self.observationIndex(x_value)
                value = self.__observations["y_values"][index]
                std = self.__observations["std_values"][index]
                histogram_data.setObservation(value, std)

        if self.hasRefcase():
            if self.hasRefcaseSample(x_value):
                index = self.refcaseIndex(x_value)
                value = self.__refcase["y_values"][index]
                histogram_data.setRefcase(value)

        for case_name in self.__case_names:
            histogram_data.addCase(case_name)

            if self.hasEnsembleSample(case_name, x_value):
                for sample in self.getEnsembleSamples(case_name, x_value):
                    histogram_data.addSample(case_name, sample)

        return histogram_data


class ReportStepLessHistogramPlotDataFactory(HistogramPlotDataFactory):
    def __init__(self, name):
        super(ReportStepLessHistogramPlotDataFactory, self).__init__(name)

    def getHistogramData(self, x_value):
        return super(ReportStepLessHistogramPlotDataFactory, self).getHistogramData(0)



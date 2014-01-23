from ert.ecl import EclSum, EclSumVector, EclSumNode
from ert.enkf.enums import ErtImplType
from ert.enkf.plot import Sample, SampleListCollection, SampleList
from ert.enkf.plot.data_fetcher import DataFetcher


class RefcaseDataFetcher(DataFetcher):
    def __init__(self, ert):
        super(RefcaseDataFetcher, self).__init__(ert)
        self.report_times = {}


    def fetchData(self):
        """ @rtype: SampleListCollection """
        return self.getRefcaseData()

    def hasRefcase(self):
        """ @rtype: bool """
        return self.ert().eclConfig().hasRefcase()

    def getRefCase(self):
        """ @rtype: EclSum """
        return self.ert().eclConfig().getRefcase()

    def getSummaryKeys(self):
        """ @rtype: StringList """
        return self.ert().ensembleConfig().getKeylistFromImplType(ErtImplType.SUMMARY)

    def __getRefcaseDataForKey(self, key):
        """ @rtype: list of Sample """
        refcase = self.getRefCase()
        vector = refcase.get_vector(key, report_only=True)

        result = []
        index = 0
        for node in vector:
            assert isinstance(node, EclSumNode)

            sample = Sample()
            sample.y = node.value
            sample.group = key
            sample.name = key
            sample.index = index
            sample.x = self.getReportStepTime(node.report_step)
            result.append(sample)

            index += 1

        return result

    def getRefcaseDataForKey(self, key):
        """ @rtype: SampleList """
        if not self.hasRefcase():
            return None

        sample_list = SampleList()
        sample_list.min_x = self.getFirstReportStepTime()
        sample_list.max_x = self.getLastReportStepTime()

        data = self.__getRefcaseDataForKey(key)

        for sample_point in data:
            sample_list.group = sample_point.group
            sample_list.addSample(sample_point)

        return sample_list



    def getRefcaseData(self):
        if not self.hasRefcase():
            return SampleListCollection()

        keys = self.getSummaryKeys()
        first_report_step_time = self.getFirstReportStepTime()
        last_report_step_time = self.getLastReportStepTime()

        result = SampleListCollection()
        for key in keys:
            data = self.__getRefcaseDataForKey(key)

            for sample_point in data:
                if not sample_point.group in result:
                    sample_list = SampleList()
                    sample_list.group = sample_point.group
                    sample_list.min_x = first_report_step_time
                    sample_list.max_x = last_report_step_time

                    result.addSampleList(sample_list)

                result[sample_point.group].addSample(sample_point)

        return result

    def getFirstReportStepTime(self):
        return self.getReportStepTime(self.getRefCase().first_report)

    def getLastReportStepTime(self):
        return self.getReportStepTime(self.getRefCase().last_report)

    def getReportStepTime(self, report_step):
        if not report_step in self.report_times:
            self.report_times[report_step] = EclSum.cNamespace().get_report_time(self.getRefCase(), report_step).ctime()

        return self.report_times[report_step]










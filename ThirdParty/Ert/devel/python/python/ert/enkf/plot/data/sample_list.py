from ert.enkf.plot.data import DictProperty, Sample, SimpleSample, SampleStatistics


class SampleList(dict):
    min_x = DictProperty("min_x")
    max_x = DictProperty("max_x")
    group = DictProperty("group")
    samples = DictProperty("samples")
    statistics = DictProperty("statistics")
    continuous_line = DictProperty("continuous_line")


    def __init__(self):
        super(SampleList, self).__init__()

        self.min_x = None
        self.max_x = None
        self.group = None
        self.samples = []
        self.statistics = SampleStatistics()
        self.continuous_line = True

    def addSample(self, sample):
        assert isinstance(sample, Sample) or isinstance(sample, SimpleSample)

        self.samples.append(sample)
        self.statistics.addSample(sample)

        if isinstance(sample, Sample):
            if sample.single_point:
                self.continuous_line = False
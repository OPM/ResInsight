from ert.enkf.plot.data import DictProperty, Sample, SimpleSample


class SampleStatistics(dict):
    min_x = DictProperty("min_x")
    max_x = DictProperty("max_x")

    min_y = DictProperty("min_y")
    max_y = DictProperty("max_y")

    min_y_with_std = DictProperty("min_y_with_std")
    max_y_with_std = DictProperty("max_y_with_std")

    def __init__(self):
        super(SampleStatistics, self).__init__()

        self.min_x = None
        self.max_x = None

        self.min_y = None
        self.max_y = None

        self.min_y_with_std = None
        self.max_y_with_std = None


    def addSample(self, sample):
        assert isinstance(sample, Sample) or isinstance(sample, SimpleSample)

        if self.min_x is None:
            self.min_x = sample.x

        if self.max_x is None:
            self.max_x = sample.x

        if self.min_y is None:
            self.min_y = sample.y

        if self.max_y is None:
            self.max_y = sample.y


        if isinstance(sample, Sample):
            if self.min_y_with_std is None:
                self.min_y_with_std = sample.y - sample.std

            if self.max_y_with_std is None:
                self.max_y_with_std = sample.y + sample.std

            self.min_y_with_std = min(self.min_y_with_std, sample.y - sample.std)
            self.max_y_with_std = max(self.max_y_with_std, sample.y + sample.std)
        else:
            if self.min_y_with_std is None:
                self.min_y_with_std = sample.y

            if self.max_y_with_std is None:
                self.max_y_with_std = sample.y

            self.min_y_with_std = min(self.min_y_with_std, sample.y)
            self.max_y_with_std = max(self.max_y_with_std, sample.y)


        self.min_x = min(self.min_x, sample.x)
        self.max_x = max(self.max_x, sample.x)

        self.min_y = min(self.min_y, sample.y)
        self.max_y = max(self.max_y, sample.y)


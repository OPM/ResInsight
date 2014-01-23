from ert.enkf.plot.data import DictProperty


class SimpleSample(dict):
    x = DictProperty("x")
    y = DictProperty("y")

    def __init__(self):
        super(SimpleSample, self).__init__()

        self.x = 0.0
        self.y = 0.0


class Sample(dict):
    index = DictProperty("index")
    x = DictProperty("x")
    y = DictProperty("y")
    std = DictProperty("std")

    group = DictProperty("group")
    name = DictProperty("name")
    single_point = DictProperty("single_point")

    def __init__(self):
        super(Sample, self).__init__()

        self.index = None
        self.x = 0.0
        self.y = 0.0
        self.std = 0.0

        self.group = None
        self.name = None

        self.single_point = False
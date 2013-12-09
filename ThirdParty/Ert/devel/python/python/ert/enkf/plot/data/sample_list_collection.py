from ert.enkf.plot.data import DictProperty, SampleList


class SampleListCollection(dict):
    # sample_lists = DictProperty("sample_lists")
    sample_lists_keys = DictProperty("sample_lists_keys")

    def __init__(self):
        super(SampleListCollection, self).__init__()

        # self.sample_lists = []
        self.sample_lists_keys = []

    def addSampleList(self, sample_list):
        assert isinstance(sample_list, SampleList)

        if sample_list.group in self:
            raise ValueError("Already exists a list for group with name: %s" % sample_list.group)

        # self.sample_lists.append(sample_list)

        self.sample_lists_keys.append(sample_list.group)
        self.sample_lists_keys.sort()
        self[sample_list.group] = sample_list



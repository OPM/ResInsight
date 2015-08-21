from ert_gui.models.mixins import BasicModelMixin


class DefaultNameFormatModel(BasicModelMixin):
    def __init__(self, default_name_format="default_%d"):
        self.__default_name_format = default_name_format
        self.__name_format = default_name_format
        super(DefaultNameFormatModel, self).__init__()


    def getValue(self):
        """ @rtype: str """
        return self.__name_format


    def setValue(self, name_format):
        if name_format is None or name_format.strip() == "" or name_format == self.__default_name_format:
            self.__name_format = self.__default_name_format
        else:
            self.__name_format = name_format

        self.observable().notify(self.VALUE_CHANGED_EVENT)

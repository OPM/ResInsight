class PlotMetricsTracker(object):
    def __init__(self):
        super(PlotMetricsTracker, self).__init__()

        self.__data_type_key = None
        self.__data_type_key_support_report_steps = False
        self.__scale_types = {}

        self.__scales = {}

    def addScaleType(self, scale_name, scale_type):
        """
         @type scale_name: str
         @type scale_type: int or float or CTime
        """
        self.__scale_types[scale_name] = scale_type


    def getType(self, scale_name):
        """
         @type scale_name: str
         @rtype: int or float or CTime or None
        """

        if scale_name is None:
            return None

        if not scale_name in self.__scale_types:
            raise KeyError("Scale type with name: '%s' does not exist!" % scale_name)

        return self.__scale_types[scale_name]


    def __createScaleTracker(self):
        scale_tracker = {}

        for scale_name in self.__scale_types.keys():
            scale_tracker[scale_name] = (None, None)

        return scale_tracker

    def setDataTypeKey(self, data_type_key):
        """ @type data_type_key: str """
        self.__data_type_key = data_type_key

        if not data_type_key in self.__scales:
            self.__scales[data_type_key] = self.__createScaleTracker()

    def getDataTypeKey(self):
        """ @rtype: str """
        return self.__data_type_key


    def setScalesForType(self, scale_name, min_value, max_value):
        if scale_name is not None:
            if not scale_name in self.__scales[self.__data_type_key]:
                raise KeyError("Scale name '%s' not registered!" % scale_name)

            self.__scales[self.__data_type_key][scale_name] = (min_value, max_value)


    def getScalesForType(self, scale_name):
        """ @rtype: tuple of (int, int) or tuple of (float, float) or tuple of (CTime, CTime) """
        scale_tracker = self.__scales[self.__data_type_key]
        if not scale_name in scale_tracker:
            return None, None

        return scale_tracker[scale_name]

    def setDataTypeKeySupportsReportSteps(self, supports_report_steps):
        self.__data_type_key_support_report_steps = supports_report_steps

    def dataTypeSupportsReportStep(self):
        return self.__data_type_key_support_report_steps

    def resetScaleType(self, scale_name):
        self.setScalesForType(scale_name, None, None)

    def hasScale(self, scale_name):
        if scale_name is None:
            return True

        scale_tracker = self.__scales[self.__data_type_key]
        return not scale_tracker[scale_name] == (None, None)

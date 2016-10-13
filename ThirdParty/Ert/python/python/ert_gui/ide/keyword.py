from ert_gui.ide.parameter import Parameter


class Keyword(object):
    def __init__(self, keyword, start, end):
        super(Keyword, self).__init__()
        #: :type: str
        self.keyword = keyword
        #: :type: int
        self.start = start
        #: :type: int
        self.end = end
        #: :type: list of Parameter
        self.parameter_list = []
        #: :type: bool
        self.error = False
        #: :type: str
        self.error_message = ""

        self.handler = None


    @property
    def length(self):
        return self.end - self.start

    def addParameter(self, value, start, end):
        self.parameter_list.append(Parameter(value, start, end))

    def parameterCount(self):
        return len(self.parameter_list)

    def parameters(self):
        """ @rtype: list of Parameter """
        return self.parameter_list

    def __getitem__(self, item):
        """ @rtype: Parameter """
        assert isinstance(item, int)
        return self.parameter_list[item]

    def hasError(self):
        """ @rtype: bool """
        parameter_error = False

        for param in self.parameter_list:
            if param.error:
                parameter_error = True

        return self.error or parameter_error



    def mergeParameters(self, from_parameter):
        parameters = self.parameter_list[from_parameter:]
        self.parameter_list = self.parameter_list[0:from_parameter - 1]

        value = " ".join([p.value for p in parameters])
        start = parameters[0].start
        end = parameters[len(parameters) - 1].end

        self.parameter_list.append(Parameter(value, start, end))

    def errorMessage(self):
        errors = []

        if self.error:
            errors.append(self.error_message)

        for parameter in self.parameter_list:
            if parameter.error:
                errors.append(parameter.error_message)

        return "\n".join(errors)

    def parameterIndexForPosition(self, position_in_block):
        index = 0

        if position_in_block <= self.end:
            return -1

        for parameter in self.parameter_list:
            if parameter.start <= position_in_block <= parameter.end:
                return index
            index += 1

        if self.parameterCount() > 0:
            if position_in_block < self.parameter_list[0].start:
                return 0
            elif position_in_block > self.parameter_list[self.parameterCount() - 1].end:
                return self.parameterCount()

        if self.parameterCount() == 0:
            return 0

        return -1
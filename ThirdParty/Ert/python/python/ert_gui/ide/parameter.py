class Parameter(object):
    def __init__(self, value, start, end):
        super(Parameter, self).__init__()
        #: :type: str
        self.value = value
        #: :type: int
        self.start = start
        #: :type: int
        self.end = end

        #: :type: bool
        self.error = False
        #: :type: str
        self.error_message = ""

    @property
    def length(self):
        """ @rtype: int """
        return self.end - self.start
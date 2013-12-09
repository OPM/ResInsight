class ValidationStatus(object):
    def __init__(self):
        super(ValidationStatus, self).__init__()
        self.reset()

    def reset(self):
        self.__fail = False
        self.__message = ""
        self.__value = None

    def setFailed(self):
        self.__fail = True

    def failed(self):
        return self.__fail

    def addToMessage(self, message):
        self.__message += message + "\n"

    def message(self):
        return self.__message.strip()

    def setValue(self, value):
        self.__value = value

    def value(self):
        return self.__value

    def __nonzero__(self):
        return not self.__fail

    def __str__(self):
        return self.__message



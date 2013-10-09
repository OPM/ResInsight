
class MessageCenter(object):
    __instance = None

    def __new__(cls):
        """ @rtype: MessageCenter """
        if cls.__instance is None:
            cls.__instance = super(MessageCenter, cls).__new__(cls)
            cls.__instance.__help_message_listeners = []
            cls.__instance.__warning_message_listeners = []

        return cls.__instance

    def addHelpMessageListeners(self, listener):
        self.__help_message_listeners.append(listener)

    def addWarningMessageListeners(self, listener):
        self.__warning_message_listeners.append(listener)

    def setHelpMessageLink(self, help_link):
        for listener in self.__help_message_listeners:
            listener.setHelpMessageLink(help_link)

    def setWarning(self, reference, warning):
        if warning is None:
            warning = ""

        for listener in self.__warning_message_listeners:
            listener.setWarning(reference, warning)


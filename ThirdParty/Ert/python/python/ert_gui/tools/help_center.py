import os


class HelpCenter(object):
    __default_help_string = "No help available!"
    __help_centers = {}

    def __init__(self, name):
        if name in HelpCenter.__help_centers:
            raise UserWarning("HelpCenter '%s' already exists!")

        super(HelpCenter, self).__init__()
        self.__name = name
        self.__listeners = []
        self.__help_prefix = ""
        self.__current_help_link = ""
        self.__help_messages = {}

        HelpCenter.__help_centers[name] = self


    def setHelpMessageLink(self, help_link):
        self.__current_help_link = help_link

        help_message = self.resolveHelpLink(help_link)

        if help_message is not None:
            self.__help_messages[help_link] = help_message
        else:
            self.__help_messages[help_link] = self.__default_help_string

        # if not help_link in self.__help_messages:
        #     help_message = self.resolveHelpLink(help_link)
        #     if help_message is not None:
        #         self.__help_messages[help_link] = help_message
        #     else:
        #         self.__help_messages[help_link] = self.__default_help_string

        for listener in self.__listeners:
            listener.setHelpMessage(help_link, self.__help_messages[help_link])

    def addListener(self, listener):
        self.__listeners.append(listener)
        help_link = self.__current_help_link
        listener.setHelpMessage(help_link, self.__help_messages[help_link])


    # The setHelpLinkPrefix should be set to point to a directory
    # containing (directories) with html help files. In the current
    # implementation this variable is set from the gert_main.py script.
    def setHelpLinkPrefix(self, prefix):
        self.__help_prefix = prefix

    def getTemplate(self):
        path = self.__help_prefix + "template.html"
        if os.path.exists(path) and os.path.isfile(path):
            f = open(path, 'r')
            template = f.read()
            f.close()
            return template
        else:
            return "<html>%s</html>"

    def resolveHelpLink(self, help_link):
        """
        Reads a HTML file from the help directory.
        The HTML must follow the specification allowed by QT here: http://doc.trolltech.com/4.6/richtext-html-subset.html
        """

        # This code can be used to find widgets with empty help labels
        #    if label.strip() == "":
        #        raise AssertionError("NOOOOOOOOOOOOOOOOOOOOO!!!!!!!!!!!!")

        path = self.__help_prefix + help_link + ".html"
        if os.path.exists(path) and os.path.isfile(path):
            f = open(path, 'r')
            help = f.read()
            f.close()
            return self.getTemplate() % help
        else:
            # This code automatically creates empty help files
            #        sys.stderr.write("Missing help file: '%s'\n" % label)
            #        if not label == "" and not label.find("/") == -1:
            #            sys.stderr.write("Creating help file: '%s'\n" % label)
            #            directory, filename = os.path.split(path)
            #
            #            if not os.path.exists(directory):
            #                os.makedirs(directory)
            #
            #            file_object = open(path, "w")
            #            file_object.write(label)
            #            file_object.close()
            return None


    @classmethod
    def getHelpCenter(cls, name):
        """ @rtype: HelpCenter """
        return HelpCenter.__help_centers.get(name)


    @staticmethod
    def addHelpToAction(action, link, help_center_name="ERT"):
        def showHelp():
            HelpCenter.getHelpCenter(help_center_name).setHelpMessageLink(link)

        action.hovered.connect(showHelp)
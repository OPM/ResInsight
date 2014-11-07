from PyQt4.QtGui import QTabWidget

class PlotPanelTracker(object):
    def __init__(self, tab_widget):
        """@type tab_widget: QTabWidget"""
        super(PlotPanelTracker, self).__init__()

        self.__selected_widget_for_type = {}
        self.__tab_widget = tab_widget

        self.__key_type_tester_map = {}

    def addKeyTypeTester(self, key_type, tester_function):
        if key_type in self.__key_type_tester_map:
            raise KeyError("Key type '%s' already exists!" % key_type)

        self.__key_type_tester_map[key_type] = tester_function


    def storePlotType(self, fetcher, key):
        """
        @type fetcher: PlotDataFetcher
        @type key: str
        """
        if key is not None:
            for key_type in self.__key_type_tester_map:
                data_type_tester_function = self.__key_type_tester_map[key_type]

                if data_type_tester_function(fetcher, key):
                    self.__selected_widget_for_type[key_type] = self.__tab_widget.currentWidget()
                    return

            raise NotImplementedError("Key '%s' not supported." % key)


    def restorePlotType(self, fetcher, key):
        """
        @type fetcher: PlotDataFetcher
        @type key: str
        """
        if key is not None:
            for key_type in self.__key_type_tester_map:
                data_type_tester_function = self.__key_type_tester_map[key_type]

                if data_type_tester_function(fetcher, key):

                    if key_type in self.__selected_widget_for_type:
                        widget = self.__selected_widget_for_type[key_type]
                        self.__tab_widget.setCurrentWidget(widget)
                    else:
                        if self.__tab_widget.count() > 0:
                            self.__tab_widget.setCurrentIndex(0)

                    return

            raise NotImplementedError("Key '%s' not supported." % key)

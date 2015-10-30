import itertools


class PlotConfig(object):

    def __init__(self, title="Unnamed", x_label=None, y_label=None):
        super(PlotConfig, self).__init__()
        self.__title = title

        self.__line_color_cycle = itertools.cycle(["#000000"]) #Black
        # Blueish, Greenlike, Beigeoid, Pinkness, Orangy-Brown
        self.setLineColorCycle(["#386CB0", "#7FC97F", "#FDC086", "#F0027F", "#BF5B17"])

        self.__legend_items = []
        self.__legend_labels = []

        self.__x_label = x_label
        self.__y_label = y_label


        self.__line_color = None
        self.__line_style = "-"
        self.__line_alpha = 0.8
        self.__line_marker = None

        self.__observations_enabled = True
        self.__observations_color = "#000000" #Black
        self.__observations_alpha = 1.0

        self.__refcase_enabled = True
        self.__refcase_color = "#000000" #Black
        self.__refcase_alpha = 0.8
        self.__refcase_style = "-"
        self.__refcase_marker = "x"
        self.__refcase_width = 2.0

        self.__legend_enabled = True
        self.__grid_enabled = True
        self.__date_support_active = True


    def lineColor(self):
        if self.__line_color is None:
            self.nextColor()

        return self.__line_color

    def nextColor(self):
        self.__line_color = self.__line_color_cycle.next()
        return self.__line_color

    def setLineColorCycle(self, color_list):
        self.__line_color_cycle = itertools.cycle(color_list)

    def addLegendItem(self, label, item):
        self.__legend_items.append(item)
        self.__legend_labels.append(label)

    def title(self):
        """ :rtype: str """
        return self.__title

    def lineStyle(self):
        return self.__line_style

    def lineAlpha(self):
        return self.__line_alpha

    def lineMarker(self):
        return self.__line_marker

    def xLabel(self):
        return self.__x_label

    def yLabel(self):
        return self.__y_label

    def legendItems(self):
        return self.__legend_items

    def legendLabels(self):
        return self.__legend_labels

    def setXLabel(self, label):
        self.__x_label = label

    def setYLabel(self, label):
        self.__y_label = label


    def setObservationsEnabled(self, enabled):
        self.__observations_enabled = enabled

    def isObservationsEnabled(self):
        return self.__observations_enabled

    def observationsColor(self):
        return self.__observations_color

    def observationsAlpha(self):
        return self.__observations_alpha


    def setRefcaseEnabled(self, enabled):
        self.__refcase_enabled = enabled

    def isRefcaseEnabled(self):
        return self.__refcase_enabled

    def refcaseColor(self):
        return self.__refcase_color

    def refcaseAlpha(self):
        return self.__refcase_alpha

    def refcaseStyle(self):
        return self.__refcase_style

    def refcaseMarker(self):
        return self.__refcase_marker

    def refcaseWidth(self):
        return self.__refcase_width


    def isLegendEnabled(self):
        return self.__legend_enabled

    def setLegendEnabled(self, enabled):
        self.__legend_enabled = enabled


    def isGridEnabled(self):
        return self.__grid_enabled

    def setGridEnabled(self, enabled):
        self.__grid_enabled = enabled

    def deactiveDateSupport(self):
        self.__date_support_active = False

    def isDateSupportActive(self):
        return self.__date_support_active

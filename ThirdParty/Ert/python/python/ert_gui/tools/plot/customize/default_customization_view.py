from ert_gui.tools.plot.customize import CustomizationView, WidgetProperty


class DefaultCustomizationView(CustomizationView):

    title = WidgetProperty()
    x_label = WidgetProperty()
    y_label = WidgetProperty()
    legend = WidgetProperty()
    grid = WidgetProperty()
    refcase = WidgetProperty()
    history = WidgetProperty()
    observations = WidgetProperty()


    def __init__(self):
        CustomizationView.__init__(self)
        label_msg = "Set to empty to use the default %s.\n" \
                    "It is also possible to use LaTeX. Enclose expression with $...$ for example: \n" \
                    "$\\alpha > \\beta$\n" \
                    "$r^3$\n" \
                    "$\\frac{1}{x}$\n" \
                    "$\\sqrt{2}$"

        self.addLineEdit("title", "Title", "The title of the plot. %s" % (label_msg % "title"), placeholder="Title")
        self.addSpacing()
        self.addLineEdit("x_label", "X Label", "The label of the X axis. %s" % (label_msg % "label"), placeholder="X Label")
        self.addLineEdit("y_label", "Y Label", "The label of the Y axis. %s" % (label_msg % "label"), placeholder="Y Label")
        self.addSpacing()
        self.addCheckBox("legend", "Legend", "Toggle Legend visibility.")
        self.addCheckBox("grid", "Grid", "Toggle Grid visibility.")
        self.addCheckBox("refcase", "Refcase", "Toggle Refcase visibility.")
        self.addCheckBox("history", "History", "Toggle History visibility.")
        self.addCheckBox("observations", "Observations", "Toggle Observations visibility.")


    def applyCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        plot_config.setTitle(self.title)

        plot_config.setXLabel(self.x_label)
        plot_config.setYLabel(self.y_label)

        plot_config.setLegendEnabled(self.legend)
        plot_config.setGridEnabled(self.grid)
        plot_config.setRefcaseEnabled(self.refcase)
        plot_config.setHistoryEnabled(self.history)
        plot_config.setObservationsEnabled(self.observations)


    def revertCustomization(self, plot_config):
        """
        @type plot_config: ert_gui.plottery.PlotConfig
        """
        if not plot_config.isUnnamed():
            self.title = plot_config.title()
        else:
            self.title = ""

        self.x_label = plot_config.xLabel()
        self.y_label = plot_config.yLabel()

        self.legend = plot_config.isLegendEnabled()
        self.grid = plot_config.isGridEnabled()
        self.refcase = plot_config.isRefcaseEnabled()
        self.history = plot_config.isHistoryEnabled()
        self.observations = plot_config.isObservationsEnabled()

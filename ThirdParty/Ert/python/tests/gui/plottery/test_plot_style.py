import datetime

from ert.test import ExtendedTestCase
from ert_gui.plottery import PlotStyle, PlotConfig, PlotLimits


class PlotStyleTest(ExtendedTestCase):

    def test_plot_style_test_defaults(self):
        style = PlotStyle("Test")

        self.assertEqual(style.name, "Test")
        self.assertEqual(style.color, "#000000")
        self.assertEqual(style.line_style, "-")
        self.assertEqual(style.alpha, 1.0)
        self.assertEqual(style.marker, "")
        self.assertEqual(style.width, 1.0)
        self.assertEqual(style.size, 7.5)
        self.assertTrue(style.isEnabled())

        style.line_style = None
        style.marker = None

        self.assertEqual(style.line_style, "")
        self.assertEqual(style.marker, "")


    def test_plot_style_builtin_checks(self):
        style = PlotStyle("Test")

        style.name = None
        self.assertIsNone(style.name)

        style.color = "notacolor"
        self.assertEqual(style.color, "notacolor") # maybe make this a proper check in future ?

        style.line_style = None
        self.assertEqual(style.line_style, "")

        style.marker = None
        self.assertEqual(style.marker, "")

        style.width = -1
        self.assertEqual(style.width, 0.0)

        style.size = -1
        self.assertEqual(style.size, 0.0)

        style.alpha = 1.1
        self.assertEqual(style.alpha, 1.0)

        style.alpha = -0.1
        self.assertEqual(style.alpha, 0.0)

        style.setEnabled(False)
        self.assertFalse(style.isEnabled())


    def test_plot_style_copy_style(self):
        style = PlotStyle("Test", "red", 0.5, ".", "o", 2.5)
        style.setEnabled(False)

        copy_style = PlotStyle("Copy")

        copy_style.copyStyleFrom(style)

        self.assertNotEqual(style.name, copy_style.name)
        self.assertEqual(style.color, copy_style.color)
        self.assertEqual(style.alpha, copy_style.alpha)
        self.assertEqual(style.line_style, copy_style.line_style)
        self.assertEqual(style.marker, copy_style.marker)
        self.assertEqual(style.width, copy_style.width)
        self.assertEqual(style.size, copy_style.size)
        self.assertNotEqual(style.isEnabled(), copy_style.isEnabled())

        another_copy_style = PlotStyle("Another Copy")
        another_copy_style.copyStyleFrom(style, copy_enabled_state=True)
        self.assertEqual(style.isEnabled(), another_copy_style.isEnabled())


    def test_plot_config(self):
        plot_config = PlotConfig("Golden Sample", x_label="x", y_label="y")

        limits = PlotLimits()
        limits.count_limits = 1, 2
        limits.depth_limits = 3, 4
        limits.density_limits = 5, 6
        limits.date_limits = datetime.date(2005, 2, 5), datetime.date(2006, 2, 6)
        limits.index_limits = 7, 8
        limits.value_limits = 9.0, 10.0

        plot_config.limits = limits
        self.assertEqual(plot_config.limits, limits)

        plot_config.setDistributionLineEnabled(True)
        plot_config.setLegendEnabled(False)
        plot_config.setGridEnabled(False)
        plot_config.setRefcaseEnabled(False)
        plot_config.setObservationsEnabled(False)

        style = PlotStyle("test_style", line_style=".", marker="g", width=2.5, size=7.5)

        plot_config.setDefaultStyle(style)
        plot_config.setRefcaseStyle(style)
        plot_config.setStatisticsStyle("mean", style)
        plot_config.setStatisticsStyle("min-max", style)
        plot_config.setStatisticsStyle("p50", style)
        plot_config.setStatisticsStyle("p10-p90", style)
        plot_config.setStatisticsStyle("p33-p67", style)
        plot_config.setStatisticsStyle("std", style)

        copy_of_plot_config = PlotConfig("Copy of Golden Sample")
        copy_of_plot_config.copyConfigFrom(plot_config)

        self.assertEqual(plot_config.isLegendEnabled(), copy_of_plot_config.isLegendEnabled())
        self.assertEqual(plot_config.isGridEnabled(), copy_of_plot_config.isGridEnabled())
        self.assertEqual(plot_config.isObservationsEnabled(), copy_of_plot_config.isObservationsEnabled())
        self.assertEqual(plot_config.isDistributionLineEnabled(), copy_of_plot_config.isDistributionLineEnabled())

        self.assertEqual(plot_config.refcaseStyle(), copy_of_plot_config.refcaseStyle())
        self.assertEqual(plot_config.observationsStyle(), copy_of_plot_config.observationsStyle())

        self.assertEqual(plot_config.histogramStyle(), copy_of_plot_config.histogramStyle())
        self.assertEqual(plot_config.defaultStyle(), copy_of_plot_config.defaultStyle())
        self.assertEqual(plot_config.currentColor(), copy_of_plot_config.currentColor())

        self.assertEqual(plot_config.getStatisticsStyle("mean"), copy_of_plot_config.getStatisticsStyle("mean"))
        self.assertEqual(plot_config.getStatisticsStyle("min-max"), copy_of_plot_config.getStatisticsStyle("min-max"))
        self.assertEqual(plot_config.getStatisticsStyle("p50"), copy_of_plot_config.getStatisticsStyle("p50"))
        self.assertEqual(plot_config.getStatisticsStyle("p10-p90"), copy_of_plot_config.getStatisticsStyle("p10-p90"))
        self.assertEqual(plot_config.getStatisticsStyle("p33-p67"), copy_of_plot_config.getStatisticsStyle("p33-p67"))
        self.assertEqual(plot_config.getStatisticsStyle("std"), copy_of_plot_config.getStatisticsStyle("std"))

        self.assertEqual(plot_config.title(), copy_of_plot_config.title())

        self.assertEqual(plot_config.limits, copy_of_plot_config.limits)


        plot_config.currentColor()  # cycle state will not be copied
        plot_config.nextColor()

        copy_of_plot_config = PlotConfig("Another Copy of Golden Sample")
        copy_of_plot_config.copyConfigFrom(plot_config)

        self.assertEqual(plot_config.refcaseStyle(), copy_of_plot_config.refcaseStyle())
        self.assertEqual(plot_config.observationsStyle(), copy_of_plot_config.observationsStyle())

        self.assertNotEqual(plot_config.histogramStyle(), copy_of_plot_config.histogramStyle())
        self.assertNotEqual(plot_config.defaultStyle(), copy_of_plot_config.defaultStyle())
        self.assertNotEqual(plot_config.currentColor(), copy_of_plot_config.currentColor())

        self.assertNotEqual(plot_config.getStatisticsStyle("mean"), copy_of_plot_config.getStatisticsStyle("mean"))
        self.assertNotEqual(plot_config.getStatisticsStyle("min-max"), copy_of_plot_config.getStatisticsStyle("min-max"))
        self.assertNotEqual(plot_config.getStatisticsStyle("p50"), copy_of_plot_config.getStatisticsStyle("p50"))
        self.assertNotEqual(plot_config.getStatisticsStyle("p10-p90"), copy_of_plot_config.getStatisticsStyle("p10-p90"))
        self.assertNotEqual(plot_config.getStatisticsStyle("p33-p67"), copy_of_plot_config.getStatisticsStyle("p33-p67"))
        self.assertNotEqual(plot_config.getStatisticsStyle("std"), copy_of_plot_config.getStatisticsStyle("std"))






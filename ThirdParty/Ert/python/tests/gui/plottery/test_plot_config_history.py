from ert.test import ExtendedTestCase
from ert_gui.plottery import PlotConfig, PlotConfigHistory


class PlotConfigHistoryTest(ExtendedTestCase):

    def test_plot_config_history(self):
        test_pc = PlotConfig("test_1")
        history = PlotConfigHistory("test", test_pc)

        self.assertEqual(history.getPlotConfig().title(), test_pc.title())
        self.assertNotEqual(history.getPlotConfig(), test_pc)

        self.assertFalse(history.isUndoPossible())
        self.assertFalse(history.isRedoPossible())

        history.applyChanges(PlotConfig("test_2"))
        self.assertTrue(history.isUndoPossible())
        self.assertFalse(history.isRedoPossible())
        self.assertEqual(history.getPlotConfig().title(), "test_2")

        history.undoChanges()
        self.assertFalse(history.isUndoPossible())
        self.assertTrue(history.isRedoPossible())
        self.assertEqual(history.getPlotConfig().title(), "test_1")

        history.redoChanges()
        self.assertTrue(history.isUndoPossible())
        self.assertFalse(history.isRedoPossible())
        self.assertEqual(history.getPlotConfig().title(), "test_2")

        history.resetChanges()
        self.assertTrue(history.isUndoPossible())
        self.assertFalse(history.isRedoPossible())
        self.assertEqual(history.getPlotConfig().title(), "test_1")

        history.undoChanges()
        self.assertTrue(history.isUndoPossible())
        self.assertTrue(history.isRedoPossible())
        self.assertEqual(history.getPlotConfig().title(), "test_2")





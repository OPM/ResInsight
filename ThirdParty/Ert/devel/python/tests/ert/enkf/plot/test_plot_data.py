from ert.enkf.plot_data import PlotBlockData, PlotBlockDataLoader, PlotBlockVector
from ert.util import DoubleVector
from ert.test import ExtendedTestCase, ErtTestContext


class PlotDataTest(ExtendedTestCase):

    def setUp(self):
        self.config_file = self.createTestPath("Statoil/config/with_RFT/config")

    def test_plot_block_vector(self):
        vector = DoubleVector()
        vector.append(1.5)
        vector.append(2.5)
        vector.append(3.5)
        plot_block_vector = PlotBlockVector(1, vector)

        self.assertEqual(plot_block_vector.getRealizationNumber(), 1)
        self.assertEqual(plot_block_vector[0], 1.5)
        self.assertEqual(plot_block_vector[2], 3.5)

        self.assertEqual(len(plot_block_vector), len(vector))


    def test_plot_block_data(self):
        depth = DoubleVector()
        depth.append(2.5)
        depth.append(3.5)

        data = PlotBlockData(depth)

        self.assertEqual(data.getDepth(), depth)

        vector = PlotBlockVector(1, DoubleVector())
        data.addPlotBlockVector(vector)
        data.addPlotBlockVector(PlotBlockVector(2, DoubleVector()))

        self.assertEqual(len(data), 2)

        self.assertEqual(vector, data[1])


    def compareLists(self, source, target):
        self.assertEqual(len(source), len(target))
        for index, value in enumerate(source):
            self.assertEqual(value, target[index])


    def checkBlockData(self, ert, obs_key, report_step):
        """
        @type ert: EnKFMain
        @type obs_key: str
        @type report_step: int
        """
        enkf_obs = ert.getObservations()
        obs_vector = enkf_obs[obs_key]
        loader = PlotBlockDataLoader(obs_vector)

        fs = ert.getEnkfFsManager().getCurrentFileSystem()
        plot_block_data = loader.load(fs, report_step)

        self.assertEqual(ert.getEnsembleSize(), len(plot_block_data))

        depth = plot_block_data.getDepth()

        depth_test_values = [1752.24998474, 1757.88926697, 1760.70924377]
        if report_step == 56:
            depth_test_values.append(1763.52885437)

        self.assertAlmostEqualList(depth_test_values, depth)

        block_obs = len(obs_vector.getNode(report_step))
        self.assertEqual(block_obs, len(plot_block_data[0]))
        self.assertEqual(block_obs, len(plot_block_data[9]))


        if report_step == 50:
            rft_values = [244.681655884, 245.217041016, 245.48500061]
        else:
            rft_values = [239.7550354, 240.290313721, 240.558197021, 240.825881958]

        self.assertAlmostEqualList(rft_values, plot_block_data[0])


        if report_step == 50:
            rft_values = [238.702560425, 239.237838745, 239.505737305]
        else:
            rft_values = [234.41583252, 234.95098877, 235.218841553, 235.486480713]

        self.assertAlmostEqualList(rft_values, plot_block_data[9])




    def test_plot_block_data_fs(self):
        with ErtTestContext("plot_block_data_test", self.config_file) as test_context:
            ert = test_context.getErt()

            self.checkBlockData(ert, "RFT2", 50)
            self.checkBlockData(ert, "RFT5", 56)


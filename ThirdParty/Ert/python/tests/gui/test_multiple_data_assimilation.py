from ert.test import ExtendedTestCase
from ert_gui.simulation.models import MultipleDataAssimilation as mda


class MDAWeightsTest(ExtendedTestCase):

    def test_weights(self):
        
        weights = mda.parseWeights("2, 2, 2, 2")
        print(weights)
        self.assertAlmostEqualList([2, 2, 2, 2], weights)

        weights = mda.parseWeights("1, 2, 3, ")
        self.assertAlmostEqualList([1, 2, 3], weights)

        weights = mda.parseWeights("1, 0, 1")
        self.assertAlmostEqualList([1, 1], weights)

        weights = mda.parseWeights("1.414213562373095, 1.414213562373095")
        self.assertAlmostEqualList([1.414213562373095, 1.414213562373095], weights)
  
        with self.assertRaises(ValueError):
            mda.parseWeights("2, error, 2, 2")
  
  
    def test_normalized_weights(self):
        
        weights = mda.normalizeWeights([1])
        self.assertAlmostEqualList([1.0], weights)
         
        weights = mda.normalizeWeights([1, 1])
        self.assertAlmostEqualList([1.414214, 1.414214], weights)

        weights = mda.normalizeWeights([1, 0, 1])
        self.assertAlmostEqualList([1.414214, 1.414214], weights)
         
        weights = mda.normalizeWeights([1, 1, 1])
        self.assertAlmostEqualList([1.732051, 1.732051, 1.732051], weights)
         
        weights = mda.normalizeWeights([8, 4, 2, 1])
        self.assertAlmostEqualList([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109], weights)
         
        weights = mda.normalizeWeights([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109])
        self.assertAlmostEqualList([9.219544457292887, 4.6097722286464435, 2.3048861143232218, 1.1524430571616109], weights)

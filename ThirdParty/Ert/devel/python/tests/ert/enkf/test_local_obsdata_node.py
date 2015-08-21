from ert.enkf import LocalObsdataNode
from ert.test import ExtendedTestCase


class LocalObsdataNodeTest(ExtendedTestCase):
    def setUp(self):
        pass

    def test_tstep(self):
        node = LocalObsdataNode("KEY")
        self.assertTrue( node.allTimeStepActive() )
        
        with self.assertRaises(ValueError):
            tstep_list = node.getStepList()

        node.addTimeStep(10)
        self.assertFalse( node.allTimeStepActive() )

        tstep_list = node.getStepList()
        self.assertEqual( len(tstep_list) , 1 )
        self.assertEqual( tstep_list[0] , 10)

        


from ert.util import Matrix
from unittest2 import TestCase

class MatrixTest(TestCase):
    def test_matrix(self):
        m = Matrix(2, 2)

        self.assertEqual(m[(0, 0)], 0)

        m[(1, 1)] = 1.5
        self.assertEqual(m[(1, 1)], 1.5)

        m[1,0] = 5
        self.assertEqual(m[1, 0], 5)

        with self.assertRaises(TypeError):
            m[5] = 5

        #todo: random crashes happens if we do this...
        #m[2, 2] = 2 # no exception raised...



from ert.util import Matrix
from ert_tests import ExtendedTestCase

class MatrixTest(ExtendedTestCase):
    def test_matrix(self):
        m = Matrix(2, 3)

        self.assertEqual(m[(0, 0)], 0)

        m[(1, 1)] = 1.5
        self.assertEqual(m[(1, 1)], 1.5)

        m[1,0] = 5
        self.assertEqual(m[1, 0], 5)

        with self.assertRaises(TypeError):
            m[5] = 5

        with self.assertRaises(IndexError):
            m[2, 0] = 0

        with self.assertRaises(IndexError):
            m[0, 3] = 0


        #todo: random crashes happens if we do this...
        #m[2, 2] = 2 # no exception raised...



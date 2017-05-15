from ecl.util import Matrix , RandomNumberGenerator
from ecl.util.enums import RngAlgTypeEnum, RngInitModeEnum
from ecl.test import ExtendedTestCase, TestAreaContext

class MatrixTest(ExtendedTestCase):
    def test_matrix(self):
        m = Matrix(2, 3)

        self.assertEqual(m.rows(), 2)
        self.assertEqual(m.columns(), 3)

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

    def test_matrix_set(self):
        m1 = Matrix(2,2)
        m1.setAll(99)
        self.assertEqual( 99 , m1[0,0] )
        self.assertEqual( 99 , m1[1,1] )
        m2 = Matrix(2,2 , value = 99)
        self.assertEqual(m1,m2)


    def test_matrix_random_init(self):
        m = Matrix(10,10)
        rng = RandomNumberGenerator(RngAlgTypeEnum.MZRAN, RngInitModeEnum.INIT_DEFAULT)
        m.randomInit( rng )

    def test_matrix_copy_column(self):
        m = Matrix(10,2)
        rng = RandomNumberGenerator(RngAlgTypeEnum.MZRAN, RngInitModeEnum.INIT_DEFAULT)
        m.randomInit( rng )

        with self.assertRaises(ValueError):
            m.copyColumn(0,2)

        with self.assertRaises(ValueError):
            m.copyColumn(2,0)

        with self.assertRaises(ValueError):
            m.copyColumn(-2,0)
            
        m.copyColumn(1, 0)
        for i in range(m.rows()):
            self.assertEqual( m[i,0] , m[i,1] )

    
    def test_matrix_scale(self):
        m = Matrix(2,2 , value = 1)
        m.scaleColumn(0 , 2)
        self.assertEqual(2 , m[0,0])
        self.assertEqual(2 , m[1,0])
        
        m.setAll(1)
        m.scaleRow(1 , 2 )
        self.assertEqual(2 , m[1,0])
        self.assertEqual(2 , m[1,1])

        with self.assertRaises(IndexError):
            m.scaleColumn(10 , 99)
        
        with self.assertRaises(IndexError):
            m.scaleRow(10 , 99)




    def test_matrix_equality(self):
        m = Matrix(2, 2)
        m[0, 0] = 2
        m[1, 1] = 4

        s = Matrix(2, 3)
        s[0, 0] = 2
        s[1, 1] = 4

        self.assertNotEqual(m, s)

        r = Matrix(2, 2)
        r[0, 0] = 2
        r[1, 1] = 3

        self.assertNotEqual(m, r)

        r[1, 1] = 4

        self.assertEqual(m, r)

    def test_str(self):
        m = Matrix(2, 2)
        s = "%s" % m

        m[0,0] = 0
        m[0,1] = 1
        m[1,0] = 2
        m[1,1] = 3
        
        with TestAreaContext("matrix_fprint"):
            with open("matrix.txt", "w") as f:
                m.fprint( f )

            with open("matrix.txt") as f:
                l1 = [ float(x) for x in f.readline().split()]
                l2 = [ float(x) for x in f.readline().split()]

            self.assertEqual( l1[0] , m[0,0])
            self.assertEqual( l1[1] , m[0,1])
            self.assertEqual( l2[0] , m[1,0])
            self.assertEqual( l2[1] , m[1,1])

            
    def test_copy_equal(self):
        m1 = Matrix(2, 2)
        m1[0,0] = 0
        m1[0,1] = 1
        m1[1,0] = 2
        m1[1,1] = 3

        m2 = m1.copy( )
        self.assertTrue( m1 == m2 )
        
    def test_sub_copy(self):
        m1 = Matrix(3,3)
        rng = RandomNumberGenerator(RngAlgTypeEnum.MZRAN, RngInitModeEnum.INIT_DEFAULT)
        m1.randomInit( rng )

        with self.assertRaises(ValueError):
            m2 = m1.subCopy( 0,0,4,2 )
            
        with self.assertRaises(ValueError):
            m2 = m1.subCopy( 0,0,2,4 )

        with self.assertRaises(ValueError):
            m2 = m1.subCopy( 4,0,1,1 )

        with self.assertRaises(ValueError):
            m2 = m1.subCopy( 0,2,1,2 )

            
        m2 = m1.subCopy( 0,0,2,2 )
        for i in range(2):
            for j in range(2):
                self.assertEqual( m1[i,j] , m2[i,j])

                
    def test_transpose(self):
        m = Matrix(3,2)
        m[0,0] = 0
        m[1,0] = 2
        m[2,0] = 4

        m[0,1] = 1
        m[1,1] = 3
        m[2,1] = 5
        
        mt = m.transpose( ) 
        
        self.assertEqual(m[0,0] , 0)
        self.assertEqual(m[1,0] , 2)
        self.assertEqual(m[2,0] , 4)

        self.assertEqual(m[0,1] , 1)
        self.assertEqual(m[1,1] , 3)
        self.assertEqual(m[2,1] , 5)

        self.assertEqual( mt.rows() , m.columns())
        self.assertEqual( mt.columns() , m.rows())
        self.assertEqual(mt[0,0] , 0)
        self.assertEqual(mt[1,0] , 1)

        self.assertEqual(mt[0,1] , 2)
        self.assertEqual(mt[1,1] , 3)

        self.assertEqual(mt[0,2] , 4)
        self.assertEqual(mt[1,2] , 5)
        
        m.transpose( inplace = True )
        self.assertEqual( m , mt )


    def test_matmul(self):
        m1 = Matrix(3,3)
        m2 = Matrix(2,2)

        with self.assertRaises(ValueError):
            Matrix.matmul( m1 , m2 )

        m = Matrix(3,2)
        m[0,0] = 0
        m[1,0] = 2
        m[2,0] = 4

        m[0,1] = 1
        m[1,1] = 3
        m[2,1] = 5
        
        mt = m.transpose( ) 

        m2 = Matrix.matmul( m , mt )
        
        self.assertEqual( m2[0,0] , 1  )
        self.assertEqual( m2[1,1] , 13 )
        self.assertEqual( m2[2,2] , 41 )
        

    def test_csv(self):
        m = Matrix(2, 2)
        m[0, 0] = 2
        m[1, 1] = 4
        with TestAreaContext("matrix_csv"):
            m.dumpCSV("matrix.csv")

    def test_identity(self):
        m1 = Matrix.identity(1)
        self.assertEqual(m1.rows(), 1)
        self.assertEqual(m1.columns(), 1)
        self.assertEqual(m1[0,0], 1)

        with self.assertRaises(ValueError):
            Matrix.identity(0)
        with self.assertRaises(ValueError):
            Matrix.identity(-3)

        m = Matrix.identity(17)
        self.assertEqual(m.rows(), 17)
        self.assertEqual(m.columns(), 17)
        for i in range(17):
            for j in range(17):
                elt = m[i, j]
                if i == j:
                    self.assertEqual(elt, 1)
                else:
                    self.assertEqual(elt, 0)

import random
from ert.geo import Surface
from ert.test import ExtendedTestCase , TestAreaContext


class SurfaceTest(ExtendedTestCase):
    def setUp(self):
        self.surface_valid = self.createTestPath("local/geometry/surface/valid_ascii.irap")
        self.surface_short = self.createTestPath("local/geometry/surface/short_ascii.irap")
        self.surface_long  = self.createTestPath("local/geometry/surface/long_ascii.irap")
        self.surface_valid2 = self.createTestPath("local/geometry/surface/valid2_ascii.irap")
        self.surface_small = self.createTestPath("local/geometry/surface/valid_small_ascii.irap")

    def test_xyz(self):
        s = Surface(self.surface_valid2)
        self.assertEqual(s.getXYZ(i=5,j=13), s.getXYZ(idx=642))
        x,y,z = s.getXYZ(i=5,j=13)
        self.assertFloatEqual(464041.44804, x)
        self.assertFloatEqual(7336966.309535, y)
        self.assertFloatEqual(0.0051, z)
        self.assertAlmostEqualList(s.getXYZ(i=6,j=13), s.getXYZ(idx=643))
        self.assertFloatEqual(-0.0006, s.getXYZ(i=6,j=13)[2]) # z value

    def test_create_new(self):
        with self.assertRaises(ValueError):
            s = Surface(None, 1, 1, 1)
        with self.assertRaises(IOError):
            s = Surface(50, 1, 1, 1)

        # values copied from irap surface_small
        ny,nx = 20,30
        xinc,yinc = 50.0, 50.0
        xstart,ystart = 463325.5625, 7336963.5
        angle = -65.0
        s_args = (None, nx, ny, xinc, yinc, xstart, ystart, angle)
        s = Surface(*s_args)
        self.assertEqual(ny*nx, len(s))
        self.assertEqual(nx, s.getNX())
        self.assertEqual(ny, s.getNY())
        small = Surface (self.surface_small)
        self.assertTrue(small.headerEqual(s))
        valid = Surface (self.surface_valid)
        self.assertFalse(valid.headerEqual(s))

        self.assertNotEqual(s, small)
        idx = 0
        for i in range(nx):
            for j in range(ny):
                s[idx] = small[idx]
                idx += 1
        self.assertEqual(s, small)

    def test_create(self):
        with self.assertRaises(IOError):
            s = Surface("File/does/not/exist")

        with self.assertRaises(ValueError):
            s = Surface(self.surface_short)

        with self.assertRaises(ValueError):
            s = Surface(self.surface_long)

        s = Surface( self.surface_valid )

        self.assertEqual( s.getNX( ) , 49 )
        self.assertEqual( s.getNY( ) , 79 )
        self.assertEqual( len(s) , 49*79 )

        with self.assertRaises(IndexError):
            v = s[49 * 79]

        with self.assertRaises(TypeError):
            v = s["KEY"]

        self.assertEqual( s[0]  ,  0.0051 )
        self.assertEqual( s[-1] , -0.0014 )

        with self.assertRaises(IndexError):
            s[49*79] = 787

        s[0] = 10
        self.assertEqual( s[0]  ,  10 )

        s[-1] = 77
        self.assertEqual( s[len(s) - 1]  ,  77 )


    def test_write(self):
        with TestAreaContext("surface/write"):

            s0 = Surface( self.surface_valid )
            s0.write( "new_surface.irap")

            s1 = Surface( "new_surface.irap")
            self.assertTrue( s1 == s0 )

            s0[0] = 99
            self.assertFalse( s1 == s0 )



    def test_copy(self):
        with TestAreaContext("surface/copy"):
            s0 = Surface( self.surface_valid )
            s1 = s0.copy( )

            self.assertTrue( s1 == s0 )
            s1[0] = 99
            self.assertFalse( s1 == s0 )
            del s0
            self.assertEqual( s1[0] , 99)

            s2 = s1.copy( copy_data = False )
            self.assertEqual( s2[0] , 0.0 )
            self.assertEqual( s2[10] , 0.0 )
            self.assertEqual( s2[100] , 0.0 )


    def test_header_equal(self):
        s0 = Surface( self.surface_valid )
        s1 = Surface( self.surface_valid2 )
        s2 = s0.copy( )

        self.assertTrue( s0.headerEqual( s0 ))
        self.assertFalse( s0.headerEqual( s1 ))
        self.assertTrue( s0.headerEqual( s2 ))


    def test_ops(self):
        s0 = Surface( self.surface_valid )
        s0.assign(1.0)
        for v in s0:
            self.assertEqual(v , 1.0)

        s0 += 1
        for v in s0:
            self.assertEqual(v , 2.0)

        s0 *= 2
        for v in s0:
            self.assertEqual(v , 4.0)

        s1 = s0 + 4
        for v in s1:
            self.assertEqual(v , 8.0)

        s2 = Surface( self.surface_valid2 )
        with self.assertRaises(ValueError):
            s3 = s1 + s2

        s4 = s1 + s0
        for v in s4:
            self.assertEqual(v , 12.0)

        s5 = s4 / 12
        for v in s5:
            self.assertEqual(v , 1.0)


    def test_ops2(self):
        s0 = Surface( self.surface_small )
        surface_list = []
        for i in range(10):
            s = s0.copy()
            for j in range(len(s)):
                s[j] = random.random()
            surface_list.append(s)

        mean = s0.copy( copy_data = False )
        for s in surface_list:
            mean += s
        mean /= len(surface_list)

        std = s0.copy( copy_data = False )
        for s in surface_list:
            std += (s - mean) * (s - mean)
        std /= (len(surface_list) - 1)


    def test_sqrt(self):
        s0 = Surface( self.surface_small )
        s0.assign(4)
        self.assertEqual(20*30, len(s0))
        s_sqrt = s0.sqrt( )
        for i in range(len(s0)):
            self.assertEqual(s0[i] , 4)
            self.assertEqual(s_sqrt[i] , 2)
        s0.inplaceSqrt( )
        self.assertTrue( s0 == s_sqrt )


    def test_xy(self):
        ny,nx = 20,30
        xinc,yinc = 50.0, 50.0
        xstart,ystart = 463325.5625, 7336963.5
        angle = 0
        s_args = (None, nx, ny, xinc, yinc, xstart, ystart, angle)
        s = Surface(*s_args)

        xy = s.getXY(0)
        self.assertEqual((xstart, ystart), xy)

        xy = s.getXY(1)
        self.assertEqual((xstart+xinc, ystart), xy)

        xy = s.getXY(nx)
        self.assertEqual((xstart, ystart+yinc), xy)

        xy = s.getXY(-1)
        self.assertEqual((xstart+xinc*(nx-1), ystart+yinc*(ny-1)), xy)

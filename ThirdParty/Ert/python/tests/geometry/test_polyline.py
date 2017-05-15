
from ecl.geo import Polyline, GeometryTools
from ecl.geo.xyz_io import XYZIo
from ecl.test import ExtendedTestCase , TestAreaContext


class PolylineTest(ExtendedTestCase):
    def setUp(self):
        self.polyline = self.createTestPath("local/geometry/pol11.xyz")
        self.closed_polyline = self.createTestPath("local/geometry/pol8.xyz")

    def test_construction(self):
        polyline = Polyline(name="test line")

        with self.assertRaises(IndexError):
            polyline.isClosed()

        self.assertEqual(polyline.getName(), "test line")

        self.assertEqual(len(polyline), 0)

        polyline.addPoint(0, 0, 0)
        self.assertEqual(len(polyline), 1)

        polyline.addPoint(1, 1, 0)
        self.assertEqual(len(polyline), 2)

        polyline.addPoint(1, 1.5)
        self.assertEqual(len(polyline), 3)

        self.assertEqual(polyline[0], (0, 0, 0))
        self.assertEqual(polyline[1], (1, 1, 0))
        self.assertEqual(polyline[2], (1, 1.5))

        polyline.addPoint(0, 1, 0)
        self.assertFalse(polyline.isClosed())

        polyline.addPoint(0, 0, 0)
        self.assertTrue(polyline.isClosed())



    def test_construction_default(self):
        with self.assertRaises(TypeError):
            pl = Polyline( init_points = 1 )

        with self.assertRaises(TypeError):
            pl = Polyline( init_points = [1.23] )

        pl = Polyline( init_points = [(1,0) , (1,1) , (1,2)])
        self.assertEqual( len(pl) , 3 )




    def test_iteration(self):
        values = [(0, 0, 0),
                  (1, 0, 0),
                  (1, 1, 0),
                  (1, 1, 1)]

        polyline = Polyline(name="iteration line")

        for p in values:
            polyline.addPoint(*p)

        for index, point in enumerate(polyline):
            self.assertEqual(point, values[index])



    def test_read_xyz_from_file(self):
        with self.assertRaises(IOError):
            XYZIo.readXYZFile("does/not/exist.xyz")

        polyline = XYZIo.readXYZFile(self.polyline)

        self.assertEqual(polyline.getName(), "pol11.xyz")
        self.assertEqual(len(polyline), 13)
        self.assertFalse(polyline.isClosed())
        self.assertEqual(polyline[0], (390271.843750, 6606121.334396, 1441.942627))  # first point
        self.assertEqual(polyline[12], (389789.263184, 6605784.945099, 1446.627808))  # last point

        polyline = XYZIo.readXYZFile(self.closed_polyline)

        self.assertEqual(polyline.getName(), "pol8.xyz")
        self.assertEqual(len(polyline), 21)
        self.assertTrue(polyline.isClosed())
        self.assertEqual(polyline[0], (396202.413086, 6606091.935028, 1542.620972))  # first point
        self.assertEqual(polyline[20], (396202.413086, 6606091.935028, 1542.620972))  # last point


    def test_closed(self):
        pl = Polyline( init_points = [(1,0) , (1,1) , (0,2)])
        self.assertFalse( pl.isClosed() )
        pl.addPoint( 1,0 )
        self.assertEqual( 4 , len(pl) ) 
        self.assertTrue( pl.isClosed() )

        pl = Polyline( init_points = [(1,0) , (1,1) , (0,2)])
        self.assertFalse( pl.isClosed() )
        pl.assertClosed( )
        self.assertEqual( 4 , len(pl) ) 
        self.assertTrue( pl.isClosed() )
        

    def test_save(self):
        with TestAreaContext("polyline/fwrite") as work_area:
            p1 = Polyline( init_points = [(1,0) , (1,1) , (1,2)])
            p2 = Polyline( init_points = [(1,0) , (1,1) , (1,2)])
            self.assertTrue( p1 == p2 )

            XYZIo.saveXYFile(p1 , "poly.xy")
            
            p2 = XYZIo.readXYFile("poly.xy")
            self.assertTrue( p1 == p2 )
            

    def test_unzip(self):
        p2 = Polyline( init_points = [(1,0) , (1,1) , (1,2)])
        p3 = Polyline( init_points = [(1,0,1) , (1,1,2) , (1,2,3)])
        (x,y) = p2.unzip()
        self.assertEqual( x , [1,1,1])
        self.assertEqual( y , [0,1,2])

        (x,y,z) = p3.unzip()
        self.assertEqual( x , [1,1,1])
        self.assertEqual( y , [0,1,2])
        self.assertEqual( z , [1,2,3])


        with self.assertRaises(ValueError):
            (x,y,z) = p2.unzip()

        with self.assertRaises(ValueError):
            (x,y) = p3.unzip()


    def test_intersection(self):
        p1 = Polyline( init_points = [(0,0) , (1,0)])
        p2 = Polyline( init_points = [(0.5 , 0.5) , (0.5,-0.5)])
        p3 = Polyline( init_points = [(0,1) , (1,1)])

        self.assertTrue(GeometryTools.polylinesIntersect( p1 , p2 ))
        self.assertFalse( GeometryTools.polylinesIntersect( p2 , p3 ))
        self.assertFalse( GeometryTools.polylinesIntersect( p1 , p3 ))

        self.assertTrue( p1.intersects(p2) )
        self.assertTrue( p2.intersects(p1) )

        self.assertTrue( not p1.intersects(p3) )
        self.assertTrue( not p3.intersects(p1) )

        
    def test_add(self):
        l1 = Polyline( init_points = [(-1,0.5) , (0.5, 0.5)])
        l2 = Polyline( init_points = [(-1,0.5) , (0.5, 0.5)])

        l3 = l1 + l2
        self.assertEqual( len(l3) , 4 )
        self.assertEqual( l1[0] , l3[0] )
        self.assertEqual( l1[1] , l3[1] )
        self.assertEqual( l1[0] , l3[2] )
        self.assertEqual( l1[1] , l3[3] )

        l4 = l1
        l4 += l2
        self.assertEqual(l3 , l4)

        
        
    def test_extend_to_edge(self):
        bound = Polyline( init_points = [(0,0) , (1,0) , (1,1) , (0,1)] )
        l1 = Polyline( init_points = [(-1,0.5) , (0.5, 0.5)])
        l2 = Polyline( init_points = [(0.25,0.25) , (0.75, 0.75)])
        
        # Bound is not closed
        with self.assertRaises(AssertionError):
            GeometryTools.extendToEdge( bound , l1 )
            
        bound.assertClosed()
        # l1 is not fully contained in bound
        with self.assertRaises(ValueError):
            GeometryTools.extendToEdge( bound , l1 )

        l3 = GeometryTools.extendToEdge( bound , l2 )
        self.assertEqual( l3[0] , (0.00,0.00))
        self.assertEqual( l3[1] , (0.25,0.25))
        self.assertEqual( l3[2] , (0.75,0.75))
        self.assertEqual( l3[3] , (1.00,1.00))
        self.assertEqual( len(l3) , 4)
        


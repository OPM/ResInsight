import math

from ert.geo import CPolyline , Polyline
from ert.geo.xyz_io import XYZIo
from ert.test import ExtendedTestCase , TestAreaContext


class CPolylineTest(ExtendedTestCase):
    def setUp(self):
        self.polyline1 = self.createTestPath("local/geometry/pol11.xyz")
        self.polyline2 = self.createTestPath("local/geometry/pol8.xyz")
        self.polyline3 = self.createTestPath("local/geometry/pol8_noend.xyz")
        

        
    def test_construction(self):
        polyline = CPolyline()
        self.assertEqual( len(polyline) , 0 )
        
        with self.assertRaises(IOError):
            CPolyline.createFromXYZFile( "Does/not/exist" )
            
        p1 = CPolyline.createFromXYZFile( self.polyline1 )
        self.assertEqual( len(p1) , 13 )
        x,y = p1[-1]
        self.assertEqual(x , 389789.263184)
        self.assertEqual(y , 6605784.945099)

        p2 = CPolyline.createFromXYZFile( self.polyline2 )
        self.assertEqual( len(p2) , 20 )
        x,y = p2[-1]
        self.assertEqual(x , 396056.314697)
        self.assertEqual(y , 6605835.119461)

        p3 = CPolyline.createFromXYZFile( self.polyline3 )
        self.assertEqual( len(p3) , 20 )
        x,y = p3[-1]
        self.assertEqual(x , 396056.314697)
        self.assertEqual(y , 6605835.119461)


        
    def test_front(self):
        polyline = CPolyline()
        polyline.addPoint( 1 , 1 )
        polyline.addPoint( 0 , 0 , front = True )
        self.assertEqual( len(polyline) , 2 )

        x,y = polyline[0]
        self.assertEqual(x,0)
        self.assertEqual(y,0)

        x,y = polyline[1]
        self.assertEqual(x,1)
        self.assertEqual(y,1)


    def test_equal(self):
        pl1 = CPolyline(name = "Poly1" , init_points = [(0,0) , (1,1) , (2,2)])
        pl2 = CPolyline(name = "Poly2" , init_points = [(0,0) , (1,1) , (2,2)])
        pl3 = CPolyline(init_points = [(0,0) , (1,1) , (2,3)])

        self.assertEqual( pl1 , pl1 )
        self.assertEqual( pl1 , pl2 )
        self.assertFalse( pl1 == pl3 )


    def test_length(self):
        polyline = CPolyline( init_points = [(0,1)])
        self.assertEqual( polyline.segmentLength() , 0 )

        polyline = CPolyline( init_points = [(0,0) , (1,0) , (1,1) , (2,2)])
        self.assertEqual( polyline.segmentLength() , 2 + math.sqrt(2))
        

    def test_extend_to_bbox(self):
        bbox = [(0,0) , (10,0) , (10,10) , (0,10)]

        polyline = CPolyline( init_points = [(11,11) , (13,13)])
        with self.assertRaises(ValueError):
            polyline.extendToBBox( bbox , start = False )
            

        polyline = CPolyline( init_points = [(1,1) , (3,3)])

        line1 = polyline.extendToBBox( bbox , start = True )
        self.assertEqual( line1 , CPolyline( init_points = [(1,1) , (0,0)]))

        line1 = polyline.extendToBBox( bbox , start = False  )
        self.assertEqual( line1 , CPolyline( init_points = [(3,3) , (10,10)]))
        
        


    def test_item(self):
        polyline = CPolyline()
        polyline.addPoint( 10 , 20 )
        self.assertEqual( len(polyline) , 1 )

        with self.assertRaises(TypeError):
            (x,y) = polyline["KEY"]
            
        with self.assertRaises(IndexError):
            (x,y) = polyline[10]
            
        (x,y) = polyline[0]
        self.assertEqual( x , 10 )
        self.assertEqual( y , 20 )
        
        polyline.addPoint(20,20)
        (x,y) = polyline[-1]
        self.assertEqual( x , 20 )
        self.assertEqual( y , 20 )
        
        
    def test_cross_segment(self):
        polyline = CPolyline( init_points = [(0,0), (1,0) , (1,1)])
        #
        #            x
        #            |
        #            |
        #            |       
        #    x-------x
        #

        self.assertTrue(polyline.segmentIntersects( (0.5 , 0.5) , (0.5 , -0.5)))
        self.assertTrue(polyline.segmentIntersects( (0.5 , 0.5) , (1.5 , 0.5)))

        self.assertFalse(polyline.segmentIntersects( (0.5 , 0.5) , ( 0.5  , 1.5)))
        self.assertFalse(polyline.segmentIntersects( (0.5 , 0.5) , (-0.5  , 0.5)))
        self.assertFalse(polyline.segmentIntersects( (0.5 , 1.5) , ( 1.5  , 1.5)))

        self.assertTrue( polyline.segmentIntersects( (1.0 , 1.0) , ( 1.5  , 1.5)))
        self.assertTrue( polyline.segmentIntersects( ( 1.5  , 1.5) , (1.0 , 1.0)))
        self.assertTrue( polyline.segmentIntersects( ( 1  , 0) , (1.0 , 1.0)))



    def test_intersects(self):
        polyline1 = CPolyline( init_points = [(0,0), (1,0) , (1,1)])
        polyline2 = CPolyline( init_points = [(0.50,0.50) , (1.50,0.50)])
        polyline3 =  Polyline( init_points = [(0.50,0.50) , (1.50,0.50)])
        polyline4 = CPolyline( init_points = [(0.50,1.50) , (1.50,1.50)])

        self.assertTrue( polyline1.intersects( polyline2 ))
        self.assertTrue( polyline1.intersects( polyline3 ))
        self.assertFalse( polyline1.intersects( polyline4 ))
        

    def test_intersects2(self):
        polyline = CPolyline( init_points = [(2,10),(2,100)])
        self.assertTrue( polyline.intersects( polyline ))


        
    def test_name(self):
        p1 = CPolyline()
        self.assertTrue( p1.getName() is None )

        p2 = CPolyline( name = "Poly2" )
        self.assertEqual( p2.getName() , "Poly2")
        

    def test_unzip(self):
        pl = CPolyline( init_points = [(0,3) , (1,4) , (2,5)] )
        x,y = pl.unzip()
        self.assertEqual(x , [0,1,2])
        self.assertEqual(y , [3,4,5])


    

import gc

from ecl.geo import CPolylineCollection , CPolyline
from ecl.geo.xyz_io import XYZIo
from ecl.test import ExtendedTestCase , TestAreaContext
from ecl.util import DoubleVector

class CPolylineCollectionTest(ExtendedTestCase):

    def test_construction(self):
        pc = CPolylineCollection()
        self.assertEqual(len(pc) , 0)
        

    def test_add_polyline(self):
        pc = CPolylineCollection()
        pl = pc.createPolyline( name = "TestP" )
        self.assertTrue( isinstance(pl , CPolyline))
        self.assertEqual(len(pc) , 1)
        self.assertTrue( "TestP" in pc )
        
        with self.assertRaises(IndexError):
            pl = pc[2]

        p0 = pc[0]
        self.assertTrue( p0 == pl )
            
        with self.assertRaises(KeyError):
            pn = pc["missing"]
            
        pn = pc["TestP"]
        self.assertTrue( pn == pl )

        px = CPolyline( name = "TestP")
        with self.assertRaises(KeyError):
            pc.addPolyline( px )
        self.assertEqual(len(pc) , 1)
            

        p2 = CPolyline( name = "Poly2")
        pc.addPolyline( p2 )
        
        self.assertEqual( len(pc) , 2 )
        self.assertTrue( "Poly2" in pc )

        l = []
        for p in pc:
            l.append(p)
        self.assertEqual( len(pc) , 2 )


        points = [(0,1) , (1,1)]
        pc.addPolyline( points , name = "XYZ")
        self.assertTrue( "XYZ" in pc )



        
    def create_collection(self):
        collection = CPolylineCollection()
        p1 = CPolyline( name = "POLY1" , init_points = [(0,10) , (1,11) , (2,12)])
        p2 = CPolyline( name = "POLY2" , init_points = [(0,100) , (10,110) , (20,120)])
        collection.addPolyline( p1 )
        collection.addPolyline( p2 )

        tail  = p1[-1]
        self.assertEqual( tail , (2,12))
        self.assertEqual(p1.getName() , "POLY1")
        
        tail  = p2[-1]
        self.assertEqual( tail , (20,120))
        self.assertEqual(p2.getName() , "POLY2")
        
        return collection


    def test_gc_polyline(self):
        # This should test that the elements in the collection can be
        # safely accessed, even after the polyline objects p1 and p2
        # from create_collection() have gone out of scope.
        c = self.create_collection()
        v = DoubleVector(initial_size = 10000)
        
        p1 = c[0]
        tail  = p1[-1]
        self.assertEqual( tail , (2,12))
        self.assertEqual(p1.getName() , "POLY1")
        
        p2 = c[1]
        tail  = p2[-1]
        self.assertEqual( tail , (20,120))
        self.assertEqual(p2.getName() , "POLY2")


    def get_polyline(self):
        collection = self.create_collection()
        return collection[0]
        

    def test_gc_collection(self):
        p1 = self.get_polyline()
        tail  = p1[-1]
        self.assertEqual( tail , (2,12))
        self.assertEqual( p1.getName() , "POLY1")
    
    def create_coll2(self):
        coll1 = self.create_collection()
        coll2 = coll1.shallowCopy()
        coll1.addPolyline( CPolyline( name = "POLY3" , init_points = [(1,1) , (2,2) , (1,3) , (1,1)]))
        
        self.assertEqual(len(coll1) , 3)
        self.assertTrue( "POLY3" in coll1 )

        self.assertEqual(len(coll2) , 2)
        self.assertFalse( "POLY3" in coll2 )
        
        return coll2
        

    def test_shallow_copy(self):
        coll2 = self.create_coll2()
        self.assertEqual(len(coll2) , 2)
        
        p1 = coll2["POLY1"]
        tail  = p1[-1]
        self.assertEqual( tail , (2,12))
        self.assertEqual(p1.getName() , "POLY1")

        p2 = coll2["POLY2"]
        tail  = p2[-1]
        self.assertEqual( tail , (20,120))
        self.assertEqual(p2.getName() , "POLY2")
        

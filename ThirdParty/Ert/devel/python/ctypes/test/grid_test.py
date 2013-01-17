#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'grid_test.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 


import ert
import ert.ecl.ecl as ecl
import datetime
import time
import unittest
import ert
import ert.ecl.ecl as ecl
from   ert.util.tvector import DoubleVector
from   ert.util.tvector import DoubleVector

from   test_util import approx_equal, approx_equalv

egrid_file  = "data/eclipse/case/ECLIPSE.EGRID"
grid_file   = "data/eclipse/case/ECLIPSE.GRID"
grdecl_file = "data/eclipse/case/include/example_grid_sim.GRDECL"    

class GridTest( unittest.TestCase ):
    def setUp(self):
        pass


    def testGRID( self ):
        grid = ecl.EclGrid( grid_file )
        self.assertTrue( grid )


    def testEGRID( self ):
        grid = ecl.EclGrid( egrid_file )
        self.assertTrue( grid )


    def create(self , filename , load_actnum = True):
        fileH = open( filename , "r")
        specgrid = ecl.EclKW.read_grdecl( fileH , "SPECGRID" , ecl_type = ecl.ECL_INT_TYPE , strict = False)
        zcorn    = ecl.EclKW.read_grdecl( fileH , "ZCORN" )
        coord    = ecl.EclKW.read_grdecl( fileH , "COORD" )
        if load_actnum:
            actnum   = ecl.EclKW.read_grdecl( fileH , "ACTNUM" , ecl_type = ecl.ECL_INT_TYPE )
        else:
            actnum   = None
            
        mapaxes  = ecl.EclKW.read_grdecl( fileH , "MAPAXES" )
        grid = ecl.EclGrid.create( specgrid , zcorn , coord , actnum , mapaxes = mapaxes)
        return grid


    def testRect(self):
        a1 = 1.0
        a2 = 2.0
        a3 = 3.0
        grid = ecl.EclGrid.create_rectangular((9,9,9) , (a1,a2,a3))
        grid.save_EGRID( "/tmp/rect.EGRID" )
        grid2 = ecl.EclGrid( "/tmp/rect.EGRID")
        self.assertTrue( grid )
        self.assertTrue( grid2 )
        
        (x,y,z) = grid.get_xyz( ijk=(4,4,4) )
        self.assertTrue( approx_equalv( [x,y,z],[4.5 * a1,4.5*a2,4.5*a3] ))

        v = grid.cell_volume( ijk=(4,4,4) )
        self.assertTrue( approx_equal( v , a1*a2*a3 ))

        z = grid.depth( ijk = (4,4,4 ))
        self.assertTrue( approx_equal( z , 4.5*a3 ))

        g1 = grid.global_index( ijk = (2,2,2) )
        g2 = grid.global_index( ijk = (4,4,4) )
        (dx,dy,dz) = grid.distance( g2 , g1 )
        self.assertTrue( approx_equalv([dx,dy,dz],[2*a1,2*a2,2*a3] ) )

        self.assertTrue( grid.cell_contains(2.5*a1 , 2.5*a2, 2.5*a3 , ijk=(2,2,2)))

        ijk = grid.find_cell( 1.5*a1 , 2.5*a2, 3.5*a3 )
        self.assertTrue( approx_equalv( ijk , [1 , 2 , 3]))



    def testCreate(self):
        grid = self.create( grdecl_file )
        self.assertTrue( grid )


    def testACTNUM(self):
        g1 = self.create( grdecl_file )
        g2 = self.create( grdecl_file , load_actnum = False )
        self.assertTrue( g1.equal( g2 ) )


    def testTime(self):
        t0 = time.clock()
        g1 = ecl.EclGrid( egrid_file )
        t1 = time.clock()
        t = t1 - t0
        self.assertTrue( t < 1.0 )


    def testSave(self):
        g1 = ecl.EclGrid( egrid_file )

        g1.save_EGRID( "/tmp/test.EGRID" )
        g2 = ecl.EclGrid( "/tmp/test.EGRID" )
        self.assertTrue( g1.equal( g2 ) )

        g1.save_GRID( "/tmp/test.GRID" )
        g2 = ecl.EclGrid( "/tmp/test.GRID" )
        self.assertTrue( g1.equal( g2 ) )
        
        fileH = open("/tmp/test.grdecl" , "w")
        g1.save_grdecl( fileH )
        fileH.close()
        g2 = self.create( "/tmp/test.grdecl" )
        self.assertTrue( g1.equal( g2 ) )

        



# def save_grdecl(grid , grdecl_file):
#     fileH = open(grdecl_file , "w")
#     grid.save_grdecl( fileH )
#     fileH.close()
# 
#     load_grdecl( grdecl_file )
# 
# init_file   = EclFile( "data/eclipse/case/ECLIPSE.INIT" )
# egrid_file  = "data/eclipse/case/ECLIPSE.EGRID"
# grid_file   = "data/eclipse/case/ECLIPSE.GRID"
# grdecl_file = "data/eclipse/case/include/example_grid_sim.GRDECL"    
# 
# grid = load_grdecl( grdecl_file )
# grid = load_grid( grid_file )
# grid = load_egrid( egrid_file )
# 
# save_grdecl( grid , "/tmp/eclipse.grdecl" )
# 
# #print "Thickness(10,11,12): %g" % grid.cell_dz( ijk=(10,11,12) )
# #
# #permx_column = DoubleVector( -999 )
# #grid.load_column( init_file.iget_named_kw( "PERMX" , 0 ) , 5 , 5 , permx_column)
# #permx_column.printf()
# #
# #print "top2    : %g   depth(10,10,0)    : %g " % (grid.top( 10, 10) , grid.depth( ijk=(10,10,0)))
# #print "bottom2 : %g   depth(10,10,nz-1) : %g " % (grid.bottom( 10 , 10 ) , grid.depth( ijk=(10,10,grid.nz - 1)))
# #
# #kw_list = init_file[1:7]
# #print kw_list


def fast_suite():
    suite = unittest.TestSuite()
    suite.addTest( GridTest( 'testGRID' ))
    suite.addTest( GridTest( 'testEGRID' ))
    suite.addTest( GridTest( 'testCreate' ))
    suite.addTest( GridTest( 'testSave' ))
    suite.addTest( GridTest( 'testTime' ))
    suite.addTest( GridTest( 'testACTNUM') )
    suite.addTest( GridTest( 'testRect' ))
    return suite


if __name__ == "__main__":
    unittest.TextTestRunner().run( fast_suite() )

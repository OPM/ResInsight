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

from   ert.ecl.ecl import *

from   ert.util.tvector import DoubleVector
from   ert.util.tvector import DoubleVector

def load_grid( grid_file ):
    grid = ecl.EclGrid( grid_file )
    return grid


def load_egrid( egrid_file ):
    grid = ecl.EclGrid( egrid_file )
    return grid


def load_grdecl( grdecl_file ):
    fileH = open( grdecl_file , "r")
    specgrid = EclKW.read_grdecl( fileH , "SPECGRID" , ecl_type = ecl.ECL_INT_TYPE , strict = False)
    zcorn    = EclKW.read_grdecl( fileH , "ZCORN" )
    coord    = EclKW.read_grdecl( fileH , "COORD" )
    actnum   = EclKW.read_grdecl( fileH , "ACTNUM" , ecl_type = ecl.ECL_INT_TYPE )

    grid = ecl.EclGrid.create( specgrid , zcorn , coord , actnum )
    return grid


init_file   = EclFile( "data/eclipse/case/ECLIPSE.INIT" )
egrid_file  = "data/eclipse/case/ECLIPSE.EGRID"
grid_file   = "data/eclipse/case/ECLIPSE.GRID"
grdecl_file = "data/eclipse/case/include/example_grid_sim.GRDECL"    

grid = load_grdecl( grdecl_file )
grid = load_grid( grid_file )
grid = load_egrid( egrid_file )

print "Thickness(10,11,12): %g" % grid.cell_dz( ijk=(10,11,12) )

permx_column = DoubleVector( -999 )
grid.load_column( init_file.iget_named_kw( "PERMX" , 0 ) , 5 , 5 , permx_column)
permx_column.printf()

print "top2    : %g   depth(10,10,0)    : %g " % (grid.top( 10, 10) , grid.depth( ijk=(10,10,0)))
print "bottom2 : %g   depth(10,10,nz-1) : %g " % (grid.bottom( 10 , 10 ) , grid.depth( ijk=(10,10,grid.nz - 1)))

kw_list = init_file[1:7]
print kw_list

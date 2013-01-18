#!/usr/bin/env python
import ert.ecl.ecl as ecl

file = "data/eclipse/Petrel/PERM.GRDECL"
grid = "data/eclipse/Petrel/GRID.GRDECL"

kw = ecl.EclKW.grdecl_load( open(file) , "PERMX" )

gridH = open(grid)
coord = ecl.EclKW.grdecl_load( gridH , "COORD" )
actnum = ecl.EclKW.grdecl_load( gridH , "ACTNUM" , ecl_type = ecl.ECL_INT_TYPE ) 
zcorn  = ecl.EclKW.grdecl_load( gridH , "ZCORN" )


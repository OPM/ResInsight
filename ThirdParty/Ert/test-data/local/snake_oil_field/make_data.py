#!/usr/bin/env python
import os
import os.path

from ert.ecl import EclGrid, EclKW, EclTypeEnum
from ert.util import RandomNumberGenerator

# This little script is used as a one-shot operation to generate the
# grid and the corresponding PERMX and PORO fields used for this test
# case. The script itself is not used bye the test.

nx = 10
ny = 10
nz =  5
ens_size = 10


def make_grid( ):
    grid = EclGrid.createRectangular( (nx,ny,nz) , (1,1,1) )
    if not os.path.isdir("grid"):
        os.makedirs("grid")
    grid.save_EGRID("grid/CASE.EGRID")

    return grid



def make_field(rng , grid , iens):
    permx = EclKW.create( "PERMX" , grid.getGlobalSize( ) , EclTypeEnum.ECL_FLOAT_TYPE)
    permx.assign( rng.getDouble( ) )

    poro = EclKW.create( "PORO" , grid.getGlobalSize( ) , EclTypeEnum.ECL_FLOAT_TYPE)
    poro.assign( rng.getDouble( ) )

    if not os.path.isdir("fields"):
        os.makedirs("fields")

    with open("fields/permx%d.grdecl" % iens,"w") as f:
        permx.write_grdecl( f )

    with open("fields/poro%d.grdecl" % iens ,"w") as f:
        poro.write_grdecl( f )


rng = RandomNumberGenerator( )
rng.setState("ABCD6375ejascEFGHIJ")


grid = make_grid( )
for iens in range(ens_size):
    make_field(rng , grid , iens)

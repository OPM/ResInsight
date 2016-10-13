#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_deprecation.py' is part of ERT - Ensemble based Reservoir Tool.
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
import warnings
import time
import datetime

from ert.test import ExtendedTestCase, TestAreaContext
from ert.ecl import EclFile,EclGrid,EclKW,EclTypeEnum,EclGrid,EclRegion,FortIO, openFortIO,EclRFT
from ert.ecl.ecl_case import EclCase
from ert.util import BoolVector

# The class Deprecation_1_9_Test contains methods which will be marked
# as deprecated in the 1.9.x versions.

warnings.simplefilter("error" , DeprecationWarning)


class Deprecation_2_1_Test(ExtendedTestCase):
    def test_ecl_kw_classprop(self):
        with self.assertRaises(DeprecationWarning):
            int_kw = EclKW.int_kw

    def test_ecl_kw_numpy_property(self):
        kw = EclKW("TEST" , 10 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            n = kw.numpy_array

            

            

class Deprecation_2_0_Test(ExtendedTestCase):
    def test_EclGrid_dims_property(self):
        with self.assertRaises(DeprecationWarning):
            grid = EclGrid.create_rectangular( (10,20,30) , (1,1,1) )

    def test_EclFile_name_property(self):
        with TestAreaContext("name") as t:
            kw = EclKW("TEST", 3, EclTypeEnum.ECL_INT_TYPE)
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                kw.fwrite( f )

            t.sync()
            
            f = EclFile( "TEST" )
            with self.assertRaises(DeprecationWarning):
                name = f.name

    def test_EclGrid_get_corner_xyz(self):
        grid = EclGrid.createRectangular( (10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            grid.get_corner_xyz(0 , global_index = 10)
            

    def test_ecl_ecl_ecl(self):
        with self.assertRaises(DeprecationWarning):
            import ert.ecl.ecl as ecl
            

    def test_ecl_kw_fortio_size_property(self):
        kw = EclKW("KW" , 1000 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.fortio_size

            
    def test_ecl_kw_size_property(self):
        kw = EclKW("KW" , 1000 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.size
            

    def test_ecl_kw_set_name(self):
        kw = EclKW("KW" , 1000 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.set_name("HEI")

    def test_ecl_kw_get_name(self):
        kw = EclKW("KW" , 1000 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.get_name( )
            

    def test_ecl_kw_create(self):
        with self.assertRaises(DeprecationWarning):
            kw = EclKW.create("KW" , 1000 , EclTypeEnum.ECL_INT_TYPE)

            
    def test_ecl_kw_iget(self):
        kw = EclKW("PORO" , 1000 , EclTypeEnum.ECL_FLOAT_TYPE)
        with self.assertRaises(DeprecationWarning):
            value = kw.iget( 100 )


    def test_ecl_kw_numeric_property(self):
        kw = EclKW("PORO" , 1000 , EclTypeEnum.ECL_FLOAT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.numeric


    def test_ecl_kw_grdecl_load(self):
        with TestAreaContext("ecl_kw/deprecate/grdecl_load"):
            kw = EclKW("PORO" , 1000 , EclTypeEnum.ECL_FLOAT_TYPE)
            with open("PORO.grdecl" , "w") as poro_file:
                kw.write_grdecl( poro_file )

            with open("PORO.grdecl") as poro_file:
                with self.assertRaises(DeprecationWarning):
                    kw = EclKW.grdecl_load( poro_file , "PORO")


    def test_ecl_grid_dual_property(self):
        grid = EclGrid.createRectangular((10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            grid.dual_grid

            
    def test_ecl_grid_name(self):
        grid = EclGrid.createRectangular((10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            grid.name

            
    def test_ecl_grid_dims_property(self):
        grid = EclGrid.createRectangular((10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            grid.nx

        with self.assertRaises(DeprecationWarning):
            grid.ny

        with self.assertRaises(DeprecationWarning):
            grid.nz

        with self.assertRaises(DeprecationWarning):
            grid.size

        with self.assertRaises(DeprecationWarning):
            grid.nactive

    def test_ecl_grid_num_lgr(self):
        grid = EclGrid.createRectangular((10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            grid.num_lgr
            


    def test_ecl_region_name(self):
        grid = EclGrid.createRectangular((10,20,30) , (1,1,1) )
        region = EclRegion( grid , False )

        with self.assertRaises(DeprecationWarning):
            region.name = "NAME"

        with self.assertRaises(DeprecationWarning):
            region.get_name( )


    def test_ecl_case( self ):
        with self.assertRaises(DeprecationWarning):
            case = EclCase( "CASE" )


    def test_rft(self):
        rft = EclRFT("WELL" , "RFT" , datetime.date.today() , 100 )

        # Property: type
        with self.assertRaises(DeprecationWarning):
            t = rft.type

        # Property: date
        with self.assertRaises(DeprecationWarning):
            d = rft.date

        # Property: well
        with self.assertRaises(DeprecationWarning):
            d = rft.date

        # Property: size
        with self.assertRaises(DeprecationWarning):
            d = rft.size

    def test_rft_file(self):
        # These deprecations are not tested - but just recorded here.
        # Property: num_wells
        # Property: headers
        pass
    

    
            
class Deprecation_1_9_Test(ExtendedTestCase):

    def test_EclGrid_dims_property(self):
        grid = EclGrid.createRectangular( (10,20,30) , (1,1,1) )
        with self.assertRaises(DeprecationWarning):
            d = grid.dims


    def test_EclKW_min_max(self):
        kw = EclKW("TEST", 3, EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(DeprecationWarning):
            kw.min

        with self.assertRaises(DeprecationWarning):
            kw.max

        with self.assertRaises(DeprecationWarning):
            kw.min_max

    def test_EclRegion_properties(self):
        grid = EclGrid.createRectangular( (10,10,10) , (1,1,1))
        region = EclRegion( grid , False )

        with self.assertRaises(DeprecationWarning):
            region.active_size

        with self.assertRaises(DeprecationWarning):
            region.global_size

        with self.assertRaises(DeprecationWarning):
            region.global_list

        with self.assertRaises(DeprecationWarning):
            region.active_list


    def test_BoolVector_active_mask(self):
        with self.assertRaises(DeprecationWarning):
            active_vector = BoolVector.active_mask("1,1,1,1,1,1")


    

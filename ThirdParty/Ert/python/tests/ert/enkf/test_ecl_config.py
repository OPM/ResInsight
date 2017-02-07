#!/usr/bin/env python
#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

import os.path
from ert.enkf import EclConfig
from ert.test import ExtendedTestCase
from ert.util import UIReturn
from ert.ecl  import EclSum

EGRID_file    = "Statoil/ECLIPSE/Gurbat/ECLIPSE.EGRID"
SMSPEC_file   = "Statoil/ECLIPSE/Gurbat/ECLIPSE.SMSPEC"
DATA_file     = "Statoil/ECLIPSE/Gurbat/ECLIPSE.DATA"
INIT_file     = "Statoil/ECLIPSE/Gurbat/EQUIL.INC"
DATA_INIT_file= "Statoil/ECLIPSE/Gurbat/ECLIPSE_INIT.DATA"
SCHEDULE_file = "Statoil/ECLIPSE/Gurbat/target.SCH"


class EclConfigTest(ExtendedTestCase):


    def test_grid(self):
        grid_file = self.createTestPath( EGRID_file )
        smspec_file = self.createTestPath( SMSPEC_file )
        ec = EclConfig()
        ui = ec.validateGridFile( grid_file )
        self.assertTrue( ui )
        self.assertTrue( isinstance(ui , UIReturn ))

        ui = ec.validateGridFile( "Does/Not/Exist" )
        self.assertFalse( ui )
        
        self.assertTrue( os.path.exists( smspec_file )) 
        ui = ec.validateGridFile( smspec_file )
        self.assertFalse( ui )



    def test_eclbase(self):
        ec = EclConfig()
        ui = ec.validateEclBase( "MixedCase%d" )
        self.assertFalse( ui )

        ui = ec.validateEclBase( "CASE%s" )
        self.assertFalse( ui )

        ui = ec.validateEclBase( "CASE%d" )
        self.assertTrue( ui )
        ec.setEclBase("CASE%d")
        self.assertEqual( "CASE%d" , ec.getEclBase())
        
    

    def test_datafile(self):
        ec = EclConfig()
        ui = ec.validateDataFile( "DoesNotExist" )
        self.assertFalse( ui )

        dfile = self.createTestPath( DATA_file )
        ui = ec.validateDataFile( dfile )
        self.assertTrue( ui )
        ec.setDataFile( dfile )
        self.assertEqual( dfile , ec.getDataFile() )


    def test_schedule_file(self):
        ec = EclConfig()
        ui = ec.validateScheduleFile( "DoesNotExist" )
        self.assertFalse( ui )

        dfile = self.createTestPath( DATA_file )
        sfile = self.createTestPath( SCHEDULE_file )

        # Setting the schedule file should fail before the datafile
        # (i.e. startdate) has been set.
        ui = ec.validateScheduleFile( sfile )
        self.assertFalse( ui )

        ec.setDataFile( dfile )
        ui = ec.validateScheduleFile( sfile )
        self.assertTrue( ui )

        ec.setScheduleFile( sfile )
        self.assertEqual( sfile , ec.getScheduleFile() )


    def test_init_section(self):
        ec = EclConfig()
        dfile = self.createTestPath( DATA_file )
        difile = self.createTestPath( DATA_INIT_file )
        ifile = self.createTestPath( INIT_file )
        
        ui = ec.validateInitSection( ifile )
        self.assertFalse( ui )
        
        ec.setDataFile( dfile )
        ui = ec.validateInitSection( ifile )
        self.assertFalse( ui )

        ec.setDataFile( difile )
        ui = ec.validateInitSection( ifile )
        self.assertTrue( ui )
        ec.setInitSection( ifile )
        self.assertTrue( ifile , ec.getInitSection() )


    def test_refcase( self ):
        ec = EclConfig()
        dfile = self.createTestPath( DATA_file )

        ui = ec.validateRefcase( "Does/not/exist" )
        self.assertFalse( ui )

        ui = ec.validateRefcase( dfile )
        self.assertTrue( ui )
        ec.loadRefcase( dfile )
        refcase = ec.getRefcase()
        self.assertTrue( isinstance( refcase , EclSum ))
        refcaseName = ec.getRefcaseName() + ".DATA"
        self.assertEqual( dfile , refcaseName )
        
    

#!/usr/bin/env python
#  Copyright (C) 2014  Statoil ASA, Norway. 
#   
#  The file 'test_region_definition_statoil.py' is part of ERT - Ensemble based Reservoir Tool.
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
try:
    from unittest2 import skipIf
except ImportError:
    from unittest import skipIf

import time
from ert.ecl.faults import FaultCollection, Fault, FaultLine, FaultSegment, RegionDefinition, FaultBlockLayer
from ert.ecl import EclGrid, EclKW, EclTypeEnum
from ert.test import ExtendedTestCase
from ert.geo import Polyline



class RegionDefinitionTest(ExtendedTestCase):
    def setUp(self):
        self.grid = EclGrid( self.createTestPath("Statoil/ECLIPSE/Mariner/MARINER.EGRID"))

        with open( self.createTestPath("Statoil/ECLIPSE/Mariner/faultblock.grdecl") ) as fileH:
            self.kw = EclKW.read_grdecl( fileH , "FAULTBLK" , ecl_type = EclTypeEnum.ECL_INT_TYPE )

        self.faults = FaultCollection( self.grid , self.createTestPath("Statoil/ECLIPSE/Mariner/faults.grdecl"))
        self.poly_file1  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol1.xyz")
        self.poly_file2  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol2.xyz")
        self.poly_file3  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol3.xyz")
        self.poly_file4  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol4.xyz")
        self.poly_file5  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol5.xyz")
        self.poly_file6  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol6.xyz")
        self.poly_file7  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol7.xyz")
        self.poly_file8  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol8.xyz")
        self.poly_file9  = self.createTestPath("Statoil/ECLIPSE/Mariner/pol9.xyz")
        self.poly_file10 = self.createTestPath("Statoil/ECLIPSE/Mariner/pol10.xyz")
        self.poly_file11 = self.createTestPath("Statoil/ECLIPSE/Mariner/pol11.xyz")

        self.fault_blocks = []
        for k in range(self.grid.getNZ()):
            blocks = FaultBlockLayer( self.grid , k)
            blocks.scanKeyword( self.kw )
            self.fault_blocks.append( blocks )

        
    def test_create(self):
        with self.assertRaises(TypeError):
            regionDef = RegionDefinition( "GG" )
            
        with self.assertRaises(ValueError):
            regionDef = RegionDefinition.create( 1 , self.faults , ["Test" , 1])

        regionDef = RegionDefinition.create( 1 , self.faults , ["DF28_LC" , self.poly_file1 , self.poly_file2 ])
        
        

    def test_create(self):
        defRegion = RegionDefinition.create( 5 , self.faults , ["DF4_MC","DF15_MC","DF43_MC","DF25_LC","DF21_C","DF1_C",self.poly_file6, "DF26_MC","DF34_MCS","DF32_MC",self.poly_file5])
        region_kw = EclKW.create( "REGIONS" , self.grid.getGlobalSize() , EclTypeEnum.ECL_INT_TYPE )
        region_kw.assign( 0 )
        
        for k in range(self.grid.getNZ()):
            with self.assertRaises(NotImplementedError):
                block_list = defRegion.findInternalBlocks( self.grid , self.fault_blocks[k] )
        
        

#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'test_kw.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os
from ert.ecl import EclKW, EclTypeEnum, EclFile
from ert.util.test_area import TestAreaContext

from ert_tests import ExtendedTestCase


def copy_long():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(0, 2000)


def copy_offset():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(200, 100)


class KWTest(ExtendedTestCase):
    def test_fortio_size( self ):
        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        size = 0
        for kw in unrst_file:
            size += kw.fortio_size

        stat = os.stat(unrst_file_path)
        self.assertTrue(size == stat.st_size)


    def test_equal(self):
        kw1 = EclKW.new("TEST", 3, EclTypeEnum.ECL_CHAR_TYPE)
        kw1[0] = "Test1"
        kw1[1] = "Test13"
        kw1[2] = "Test15"

        kw2 = EclKW.new("TEST", 3, EclTypeEnum.ECL_CHAR_TYPE)
        kw2[0] = "Test1"
        kw2[1] = "Test13"
        kw2[2] = "Test15"

        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw2[2] = "Test15X"
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2))

        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        kw1 = unrst_file["PRESSURE"][0]
        kw2 = kw1.deep_copy()

        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw2 *= 1.00001
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2, epsilon=1e-8))
        self.assertTrue(kw1.equal_numeric(kw2, epsilon=1e-2))

        kw1 = unrst_file["ICON"][10]
        kw2 = kw1.deep_copy()
        self.assertTrue(kw1.equal(kw2))
        self.assertTrue(kw1.equal_numeric(kw2))

        kw1[-1] += 1
        self.assertFalse(kw1.equal(kw2))
        self.assertFalse(kw1.equal_numeric(kw2))


    def kw_test( self, data_type, data, fmt ):
        name1 = "file1.txt"
        name2 = "file2.txt"
        kw = EclKW.new("TEST", len(data), data_type)
        i = 0
        for d in data:
            kw[i] = d
            i += 1

        file1 = open(name1, "w")
        kw.fprintf_data(file1, fmt)
        file1.close()

        file2 = open(name2, "w")
        for d in data:
            file2.write(fmt % d)
        file2.close()
        self.assertFilesAreEqual(name1, name2)


    def test_sub_copy(self):
        unrst_file_path = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        unrst_file = EclFile(unrst_file_path)
        swat = unrst_file["SWAT"][0]

        swat1 = swat.sub_copy(0, -1)
        swat2 = swat.sub_copy(0, swat.size)

        self.assertTrue(swat.equal(swat1))
        self.assertTrue(swat.equal(swat2))

        swat3 = swat.sub_copy(20000, 100, new_header="swat")
        self.assertTrue(swat3.name == "swat")
        self.assertTrue(swat3.size == 100)
        equal = True
        for i in range(swat3.size):
            if swat3[i] != swat[i + 20000]:
                equal = False
        self.assertTrue(equal)

        self.assertRaises(IndexError, copy_long)
        self.assertRaises(IndexError, copy_offset)


    def test_fprintf( self ):
        #work_area = TestArea("python.ecl_kw")
        with TestAreaContext("python.ecl_kw"):
            self.kw_test(EclTypeEnum.ECL_INT_TYPE, [0, 1, 2, 3, 4, 5], "%4d\n")
            self.kw_test(EclTypeEnum.ECL_FLOAT_TYPE, [0.0, 1.1, 2.2, 3.3, 4.4, 5.5], "%12.6f\n")
            self.kw_test(EclTypeEnum.ECL_DOUBLE_TYPE, [0.0, 1.1, 2.2, 3.3, 4.4, 5.5], "%12.6f\n")
            self.kw_test(EclTypeEnum.ECL_BOOL_TYPE, [True, True, True, False, True], "%4d\n")
            self.kw_test(EclTypeEnum.ECL_CHAR_TYPE, ["1", "22", "4444", "666666", "88888888"], "%-8s\n")


#def cutoff( x , arg ):
#    if x < arg:
#        return 0
#    else:
#        return x
#
#
#init_file = ecl.EclFile( "data/eclipse/case/ECLIPSE.INIT" )
#permx     = init_file.iget_named_kw("PERMX" , 0 )
#poro      = init_file.iget_named_kw("PORO" , 0 )
#pvt       = init_file.iget_named_kw("PVTNUM" , 0 )
#grid      = ecl.EclGrid( "data/eclipse/case/ECLIPSE.EGRID" )
#
#
#poro3d = grid.create3D( poro   , default = -100)
#
#print "max:%g" % poro.max
#print "min:%g" % poro.min
#
#mask1 = ecl.EclRegion( grid , False )
#mask2 = ecl.EclRegion( grid , False )
#mask1.select_less( poro , 0.15 )
#mask2.select_more( poro , 0.30 ) 
#
#mask3  = mask1.copy()
#mask3 |= mask2
#
#mask1.reset()
#(x,y,z) = grid.get_xyz( ijk = (grid.nx / 2 , grid.ny /2 , grid.nz / 2) )
#mask1.select_above_plane( [0,0,1] , [x,y,z] )
#print mask1.active_list.size
#print mask1.active_list.str( max_lines = None )
#
#print mask2.active_list.size
#print mask3.active_list.size
#
#poro.apply( cutoff , mask = mask1 , arg = 0.05)
#
#
#poro.write_grdecl( open("/tmp/poro_cos.grdecl" , "w") )
#
#poro.add( permx , mask = mask1)
#poro.sub( 1 )
#poro.mul( poro )
#poro.assign( 14.0 )
#poro.div( 7.0 )
#
#new_p = poro / 7.0
#
#poro.write_grdecl( open("/tmp/poro_cos.grdecl" , "w") )
#
#init_file = ecl.EclFile( "data/eclipse/case/ECLIPSE.INIT" )
#permx_kw = init_file.iget_named_kw( "PERMX" , 0 )
#permx_new = ecl.EclKW.new( "PERMX" , 3 , ecl.ECL_FLOAT_TYPE )
#print ecl.ECL_GRID_FILE
#print ecl.ecl_file_enum
#permx_new[0] = 1
#permx_new[1] = 2
#permx_new[2] = 3
#
##init_file.replace_kw( permx_kw , permx_new )
#fortio = ecl.FortIO.writer( "/tmp/init" )
#print fortio
#init_file.fwrite( fortio )
#fortio.close()
#
#poro   = ecl.EclKW.read_grdecl( open( "data/eclipse/case/include/example_poro.GRDECL" , "r") , "PORO" )
#eqlnum = ecl.EclKW.read_grdecl( open( "data/eclipse/case/include/example_eqlnum.GRDECL" , "r") , "EQLNUM" )
#dummy  = ecl.EclKW.read_grdecl( open( "data/eclipse/case/include/example_eqlnum.GRDECL" , "r") , "BJARNE" , ecl_type = ecl.ECL_INT_TYPE)
#
#if dummy:
#    print "Loading BJARNE OK"
#else:
#    print "Loading BJARNE Failed (correctly)"
#
#print "Poro[100] :%g  eqlnum[100]:%d" % (poro[100] , eqlnum[100]) 
#
#if not eqlnum.type == ecl.ECL_INT_TYPE:
#    sys.exit("Type error when loading eqlnum")
#
#p2 = poro[100:160]
#if p2:
#    print p2.header
#print poro
#print pvt.str( max_lines = 8 )
#print eqlnum
#
#print ecl.EclKW.int_kw
#poro.add_int_kw("BJARNE")
#print eqlnum.int_kw

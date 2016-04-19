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
import random
from ert.ecl import EclKW, EclTypeEnum, EclFile, FortIO, EclFileFlagEnum , openFortIO

from ert.test import ExtendedTestCase , TestAreaContext


def copy_long():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(0, 2000)


def copy_offset():
    src = EclKW.create("NAME", 100, EclTypeEnum.ECL_FLOAT_TYPE)
    copy = src.sub_copy(200, 100)


class KWTest(ExtendedTestCase):

    def test_min_max(self):
        kw = EclKW("TEST", 3, EclTypeEnum.ECL_INT_TYPE)
        kw[0] = 10
        kw[1] = 5
        kw[2] = 0

        self.assertEqual( 10 , kw.getMax())
        self.assertEqual( 0  , kw.getMin())
        self.assertEqual( (0,10)  , kw.getMinMax())

        



    def kw_test( self, data_type, data, fmt ):
        name1 = "file1.txt"
        name2 = "file2.txt"
        kw = EclKW("TEST", len(data), data_type)
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
        self.assertEqual( kw.getEclType() , data_type )

    def test_create(self):
        with self.assertRaises(ValueError):
            EclKW.create( "ToGodDamnLong" , 100 , EclTypeEnum.ECL_CHAR_TYPE )



    def test_sum( self ):
        kw_string = EclKW.create( "STRING" , 100 , EclTypeEnum.ECL_CHAR_TYPE )
        with self.assertRaises(ValueError):
            kw_string.sum()


        kw_int = EclKW.create( "INT" , 4 , EclTypeEnum.ECL_INT_TYPE )
        kw_int[0] = 1
        kw_int[1] = 2
        kw_int[2] = 3
        kw_int[3] = 4
        self.assertEqual( kw_int.sum() , 10 )

        kw_d = EclKW.create( "D" , 4 , EclTypeEnum.ECL_DOUBLE_TYPE )
        kw_d[0] = 1
        kw_d[1] = 2
        kw_d[2] = 3
        kw_d[3] = 4
        self.assertEqual( kw_d.sum() , 10 )

        kw_f = EclKW.create( "F" , 4 , EclTypeEnum.ECL_FLOAT_TYPE )
        kw_f[0] = 1
        kw_f[1] = 2
        kw_f[2] = 3
        kw_f[3] = 4
        self.assertEqual( kw_f.sum() , 10 )

        kw_b = EclKW.create( "F" , 4 , EclTypeEnum.ECL_BOOL_TYPE )
        kw_b[0] = False
        kw_b[1] = True
        kw_b[2] = False
        kw_b[3] = True
        self.assertEqual( kw_b.sum() , 2 )



    def test_fprintf( self ):
        with TestAreaContext("python.ecl_kw"):
            self.kw_test(EclTypeEnum.ECL_INT_TYPE, [0, 1, 2, 3, 4, 5], "%4d\n")
            self.kw_test(EclTypeEnum.ECL_FLOAT_TYPE, [0.0, 1.1, 2.2, 3.3, 4.4, 5.5], "%12.6f\n")
            self.kw_test(EclTypeEnum.ECL_DOUBLE_TYPE, [0.0, 1.1, 2.2, 3.3, 4.4, 5.5], "%12.6f\n")
            self.kw_test(EclTypeEnum.ECL_BOOL_TYPE, [True, True, True, False, True], "%4d\n")
            self.kw_test(EclTypeEnum.ECL_CHAR_TYPE, ["1", "22", "4444", "666666", "88888888"], "%-8s\n")

    def test_kw_write(self):
        with TestAreaContext("python/ecl_kw/writing"):

            data = [random.random() for i in range(10000)]

            kw = EclKW("TEST", len(data), EclTypeEnum.ECL_DOUBLE_TYPE)
            i = 0
            for d in data:
                kw[i] = d
                i += 1

            fortio = FortIO("ECL_KW_TEST", FortIO.WRITE_MODE)
            kw.fwrite(fortio)
            fortio.close()

            fortio = FortIO("ECL_KW_TEST")

            kw2 = EclKW.fread(fortio)

            self.assertTrue(kw.equal(kw2))

            ecl_file = EclFile("ECL_KW_TEST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)
            kw3 = ecl_file["TEST"][0]
            self.assertTrue(kw.equal(kw3))
            ecl_file.save_kw(kw3)
            ecl_file.close()

            fortio = FortIO("ECL_KW_TEST", FortIO.READ_AND_WRITE_MODE)
            kw4 = EclKW.fread(fortio)
            self.assertTrue(kw.equal(kw4))
            fortio.seek(0)
            kw4.fwrite(fortio)
            fortio.close()

            ecl_file = EclFile("ECL_KW_TEST")
            kw5 = ecl_file["TEST"][0]
            self.assertTrue(kw.equal(kw5))



    def test_fprintf_data(self):
        with TestAreaContext("kw_no_header"):
            kw = EclKW.create("REGIONS" , 10 , EclTypeEnum.ECL_INT_TYPE)
            for i in range(len(kw)):
                kw[i] = i
                
            fileH = open("test" , "w")
            kw.fprintf_data( fileH )
            fileH.close()

            fileH = open("test" , "r")
            data = []
            for line in fileH.readlines():
                tmp = line.split()
                for elm in tmp:
                    data.append( int(elm) )

            for (v1,v2) in zip(data,kw):
                self.assertEqual(v1,v2)


    def test_sliced_set(self):
        kw = EclKW.create("REGIONS" , 10 , EclTypeEnum.ECL_INT_TYPE)
        kw.assign(99)
        kw[0:5] = 66
        self.assertEqual(kw[0] , 66)
        self.assertEqual(kw[4] , 66)
        self.assertEqual(kw[5] , 99)

        
    def test_long_name(self):
        with self.assertRaises(ValueError):
            EclKW.create("LONGLONGNAME" , 10 , EclTypeEnum.ECL_INT_TYPE)

        kw = EclKW.create("REGIONS" , 10 , EclTypeEnum.ECL_INT_TYPE)
        with self.assertRaises(ValueError):
            kw.set_name("LONGLONGNAME")


    def test_abs(self):
        kw = EclKW("NAME" , 10 , EclTypeEnum.ECL_CHAR_TYPE)
        with self.assertRaises(TypeError):
            abs_kw = abs(kw)

        kw = EclKW("NAME" , 10 , EclTypeEnum.ECL_BOOL_TYPE)
        with self.assertRaises(TypeError):
            abs_kw = abs(kw)

        kw = EclKW("NAME" , 10 , EclTypeEnum.ECL_INT_TYPE)
        for i in range(len(kw)):
            kw[i] = -i

        abs_kw = abs(kw)
        for i in range(len(kw)):
            self.assertEqual(kw[i] , -i ) 
            self.assertEqual(abs_kw[i] , i ) 


    def test_fmt(self):
        kw1 = EclKW( "NAME1" , 100 , EclTypeEnum.ECL_INT_TYPE)
        kw2 = EclKW( "NAME2" , 100 , EclTypeEnum.ECL_INT_TYPE)

        for i in range(len(kw1)):
            kw1[i] = i + 1
            kw2[i] = len(kw1) - kw1[i]
            
        with TestAreaContext("ecl_kw/fmt") as ta:
            with openFortIO( "TEST.FINIT" , FortIO.WRITE_MODE , fmt_file = True ) as f:
                kw1.fwrite( f )
                kw2.fwrite( f )
                
            with openFortIO( "TEST.FINIT" , fmt_file = True ) as f:
                kw1b = EclKW.fread( f )
                kw2b = EclKW.fread( f )

            self.assertTrue( kw1 == kw1b )
            self.assertTrue( kw2 == kw2b )

            f = EclFile( "TEST.FINIT" )
            self.assertTrue( kw1 == f[0] )
            self.assertTrue( kw2 == f[1] )
            

            
            

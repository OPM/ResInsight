#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'sum_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import datetime
import os.path
from unittest import skipIf

from ert.ecl import EclFile, FortIO, EclKW , openFortIO , openEclFile
from ert.ecl import EclFileFlagEnum, EclTypeEnum, EclFileEnum

from ert.test import ExtendedTestCase , TestAreaContext


    

class EclFileStatoilTest(ExtendedTestCase):
    def setUp(self):
        self.test_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.test_fmt_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.FUNRST")

    def assertFileType(self , filename , expected):
        file_type , step , fmt_file = EclFile.getFileType(filename)
        self.assertEqual( file_type , expected[0] )
        self.assertEqual( fmt_file , expected[1] )
        self.assertEqual( step , expected[2] )

        
        
    def test_restart_days(self):
        rst_file = EclFile( self.test_file )
        self.assertAlmostEqual(  0.0 , rst_file.iget_restart_sim_days(0) )
        self.assertAlmostEqual( 31.0 , rst_file.iget_restart_sim_days(1) )
        self.assertAlmostEqual( 274.0 , rst_file.iget_restart_sim_days(10) )

        with self.assertRaises(KeyError):
            rst_file.restart_get_kw("Missing" , dtime = datetime.date( 2004,1,1))

        with self.assertRaises(IndexError):
            rst_file.restart_get_kw("SWAT" , dtime = datetime.date( 1985 , 1 , 1))
            
            

    def test_iget_named(self):
        f = EclFile(self.test_file)
        N = f.num_named_kw( "SWAT" )
        with self.assertRaises(KeyError):
            s = f.iget_named_kw( "SWAT" , N + 1)



    def test_fwrite( self ):
        #work_area = TestArea("python/ecl_file/fwrite")
        with TestAreaContext("python/ecl_file/fwrite"):
            rst_file = EclFile(self.test_file)
            fortio = FortIO("ECLIPSE.UNRST", FortIO.WRITE_MODE)
            rst_file.fwrite(fortio)
            fortio.close()
            rst_file.close()
            self.assertFilesAreEqual("ECLIPSE.UNRST", self.test_file)




    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow file test skipped!")
    def test_save(self):
        #work_area = TestArea("python/ecl_file/save")
        with TestAreaContext("python/ecl_file/save", store_area=False) as work_area:
            work_area.copy_file(self.test_file)
            rst_file = EclFile("ECLIPSE.UNRST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)
            swat0 = rst_file["SWAT"][0]
            swat0.assign(0.75)
            rst_file.save_kw(swat0)
            rst_file.close()
            self.assertFilesAreNotEqual("ECLIPSE.UNRST",self.test_file)

            rst_file1 = EclFile(self.test_file)
            rst_file2 = EclFile("ECLIPSE.UNRST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)

            swat1 = rst_file1["SWAT"][0]
            swat2 = rst_file2["SWAT"][0]
            swat2.assign(swat1)

            rst_file2.save_kw(swat2)
            self.assertTrue(swat1.equal(swat2))
            rst_file1.close()
            rst_file2.close()

            # Random failure ....
            self.assertFilesAreEqual("ECLIPSE.UNRST", self.test_file)



    @skipIf(ExtendedTestCase.slowTestShouldNotRun(), "Slow file test skipped!")
    def test_save_fmt(self):
        #work_area = TestArea("python/ecl_file/save_fmt")
        with TestAreaContext("python/ecl_file/save_fmt") as work_area:
            work_area.copy_file(self.test_fmt_file)
            rst_file = EclFile("ECLIPSE.FUNRST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)
            swat0 = rst_file["SWAT"][0]
            swat0.assign(0.75)
            rst_file.save_kw(swat0)
            rst_file.close()
            self.assertFilesAreNotEqual("ECLIPSE.FUNRST", self.test_fmt_file)

            rst_file1 = EclFile(self.test_fmt_file)
            rst_file2 = EclFile("ECLIPSE.FUNRST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)

            swat1 = rst_file1["SWAT"][0]
            swat2 = rst_file2["SWAT"][0]

            swat2.assign(swat1)
            rst_file2.save_kw(swat2)
            self.assertTrue(swat1.equal(swat2))
            rst_file1.close()
            rst_file2.close()

            # Random failure ....
            self.assertFilesAreEqual("ECLIPSE.FUNRST", self.test_fmt_file)


    def test_truncated(self):
        with TestAreaContext("python/ecl_file/truncated") as work_area:
            work_area.copy_file(self.test_file)
            size = os.path.getsize("ECLIPSE.UNRST")
            with open("ECLIPSE.UNRST" , "r+") as f:
                f.truncate( size / 2 )
            
            with self.assertRaises(IOError):
                rst_file = EclFile("ECLIPSE.UNRST")

            with self.assertRaises(IOError):
                rst_file = EclFile("ECLIPSE.UNRST", flags=EclFileFlagEnum.ECL_FILE_WRITABLE)
                
                
        
            

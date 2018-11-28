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

from ecl import EclFileFlagEnum, EclFileEnum
from ecl.eclfile import EclFile, FortIO, EclKW , openFortIO , openEclFile

from ecl.util.test import TestAreaContext
from tests import EclTest, statoil_test


@statoil_test()
class EclFileStatoilTest(EclTest):
    def setUp(self):
        self.test_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")
        self.test_fmt_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.FUNRST")

    def assertFileType(self , filename , expected):
        file_type , step , fmt_file = EclFile.getFileType(filename)
        self.assertEqual( file_type , expected[0] )
        self.assertEqual( fmt_file , expected[1] )
        self.assertEqual( step , expected[2] )


    def test_fast_open(self):
        with TestAreaContext("index"):
            f0 = EclFile( self.test_file )
            f0.write_index("index")
            f1 = EclFile( self.test_file , 0 , "index")
            for kw0,kw1 in zip(f0,f1):
                self.assertEqual( kw0,kw1 )

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
        with self.assertRaises(IndexError):
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




    @skipIf(EclTest.slowTestShouldNotRun(), "Slow file test skipped!")
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



    @skipIf(EclTest.slowTestShouldNotRun(), "Slow file test skipped!")
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


    def test_restart_view(self):
        f = EclFile( self.test_file )
        with self.assertRaises(ValueError):
            v = f.restartView( )

        v = f.restartView( sim_days = 274 )
        v = f.restartView( sim_time = datetime.date( 2004,1,1) )
        v = f.restartView( report_step = 30 )
        v = f.restartView( seqnum_index = 30 )


    def test_index(self):
        with TestAreaContext("python/ecl_file/truncated"):
            f0 = EclFile( self.test_file )
            f0.write_index( "index" )

            f1 = EclFile( self.test_file , index_filename = "index")
            for kw0,kw1 in zip(f0,f1):
                self.assertEqual(kw0,kw1)


    def test_ix_case(self):
        f = EclFile( self.createTestPath( "Statoil/ECLIPSE/ix/summary/Create_Region_Around_Well.SMSPEC"))

        # Keywords
        self.assertTrue( "KEYWORDS" in f )
        keywords_loaded = list(f["KEYWORDS"][0])
        keywords_from_file = [
                'TIME', 'YEARS', 'AAQR', 'AAQT', 'AAQP', 'AAQR', 'AAQT',
                'AAQP', 'AAQR', 'AAQT', 'AAQP', 'FPPW', 'FPPO', 'FPPG', 'FNQT',
                'FNQR', 'FEIP', 'FWPT', 'FWIT', 'FWIP', 'FWGR', 'FVPT', 'FVPR',
                'FVIT', 'FVIR', 'FPR', 'FOPT', 'FOIT', 'FOIR', 'FOIPL',
                'FOIPG', 'FOIP', 'FGPT', 'FGIT', 'FGIPL', 'FGIPG', 'FGIP',
                'FAQT', 'FAQR', 'FGOR', 'FWCT', 'FGSR', 'FGIR', 'FGPR', 'FWIR',
                'FWPR', 'FOPR', 'MEMORYTS', 'NAIMFRAC', 'TCPUDAY', 'TCPUTS',
                'NBAKFL', 'NNUMST', 'NNUMFL', 'NEWTFL', 'MSUMNEWT', 'MSUMLINS',
                'MLINEARS', 'NLINEARS', 'NEWTON', 'ELAPSED', 'TCPU',
                'TIMESTEP', 'GOPR', 'GOPR', 'GOPR', 'GWPR', 'GWPR', 'GWPR',
                'GWIR', 'GWIR', 'GWIR', 'GGPR', 'GGPR', 'GGPR', 'GWCT', 'GWCT',
                'GWCT', 'GGOR', 'GGOR', 'GGOR', 'GGIR', 'GGIR', 'GGIR', 'GGIT',
                'GGIT', 'GGIT', 'GGPT', 'GGPT', 'GGPT', 'GOIR', 'GOIR', 'GOIR',
                'GOIT', 'GOIT', 'GOIT', 'GOPT', 'GOPT', 'GOPT', 'GVIR', 'GVIR',
                'GVIR', 'GVIT', 'GVIT', 'GVIT', 'GVPR', 'GVPR', 'GVPR', 'GVPT',
                'GVPT', 'GVPT', 'GWGR', 'GWGR', 'GWGR', 'GWIT', 'GWIT', 'GWIT',
                'GWPT', 'GWPT', 'GWPT', 'WOPR', 'WOPR', 'WOPR', 'WOPR', 'WOPR',
                'WOPR', 'WWPR', 'WWPR', 'WWPR', 'WWPR', 'WWPR', 'WWPR', 'WWIR',
                'WWIR', 'WWIR', 'WWIR', 'WWIR', 'WWIR', 'WGPR', 'WGPR', 'WGPR',
                'WGPR', 'WGPR', 'WGPR', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT',
                'WWCT', 'WMCTL', 'WMCTL', 'WMCTL', 'WMCTL', 'WMCTL', 'WMCTL',
                'WGOR', 'WGOR', 'WGOR', 'WGOR', 'WGOR', 'WGOR', 'WAPI', 'WAPI',
                'WAPI', 'WAPI', 'WAPI', 'WAPI', 'WBHP', 'WBHP', 'WBHP', 'WBHP',
                'WBHP', 'WBHP', 'WGIR', 'WGIR', 'WGIR', 'WGIR', 'WGIR', 'WGIR',
                'WGIT', 'WGIT', 'WGIT', 'WGIT', 'WGIT', 'WGIT', 'WGPT', 'WGPT',
                'WGPT', 'WGPT', 'WGPT', 'WGPT', 'WOIR', 'WOIR', 'WOIR', 'WOIR',
                'WOIR', 'WOIR', 'WOIT', 'WOIT', 'WOIT', 'WOIT', 'WOIT', 'WOIT',
                'WOPT', 'WOPT', 'WOPT', 'WOPT', 'WOPT', 'WOPT', 'WPIG', 'WPIG',
                'WPIG', 'WPIG', 'WPIG', 'WPIG', 'WPIO', 'WPIO', 'WPIO', 'WPIO',
                'WPIO', 'WPIO', 'WPIW', 'WPIW', 'WPIW', 'WPIW', 'WPIW', 'WPIW',
                'WTHP', 'WTHP', 'WTHP', 'WTHP', 'WTHP', 'WTHP', 'WVIR', 'WVIR',
                'WVIR', 'WVIR', 'WVIR', 'WVIR', 'WVIT', 'WVIT', 'WVIT', 'WVIT',
                'WVIT', 'WVIT', 'WVPR', 'WVPR', 'WVPR', 'WVPR', 'WVPR', 'WVPR',
                'WVPT', 'WVPT', 'WVPT', 'WVPT', 'WVPT', 'WVPT', 'WWGR', 'WWGR',
                'WWGR', 'WWGR', 'WWGR', 'WWGR', 'WWIT', 'WWIT', 'WWIT', 'WWIT',
                'WWIT', 'WWIT', 'WWPT', 'WWPT', 'WWPT', 'WWPT', 'WWPT', 'WWPT',
                'WBHT', 'WBHT', 'WBHT', 'WBHT', 'WBHT', 'WBHT', 'WBP', 'WBP',
                'WBP', 'WBP', 'WBP', 'WBP', 'WWCT', 'WWCT', 'WWCT', 'WWCT',
                'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT',
                'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT',
                'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT', 'WWCT'
                ]

        padd = lambda str_len : (lambda s : s + (" " * (max(0, str_len-len(s)))))
        self.assertEqual( list(map(padd(8), keywords_from_file)), keywords_loaded)

        # Names
        self.assertTrue( "NAMES" in f )
        names_loaded = list(f["NAMES"][0])
        names_from_file = [
                '', '', 'AQFR_1', 'AQFR_1', 'AQFR_1', 'AQFR_2', 'AQFR_2',
                'AQFR_2', 'AQFR_3', 'AQFR_3', 'AQFR_3', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD', 'FIELD',
                'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE',
                'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD',
                'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO',
                'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE',
                'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD',
                'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO',
                'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE', 'TWO', 'FIELD', 'ONE',
                'TWO', 'FIELD', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8',
                'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI',
                'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6',
                'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD',
                'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4',
                'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8',
                'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI',
                'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6',
                'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD',
                'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4',
                'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8',
                'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI',
                'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6',
                'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD',
                'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4',
                'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8',
                'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI',
                'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6',
                'I8', 'HWELL_PROD', 'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD',
                'GI', 'I2', 'I4', 'I6', 'I8', 'HWELL_PROD', 'GI', 'I2', 'I4',
                'I6', 'I8', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+',
                ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+',
                ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+',
                ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+',
                ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+',
                ':+:+:+:+', ':+:+:+:+', ':+:+:+:+', ':+:+:+:+'
                ]

        self.assertEqual( list(map(padd(10), names_from_file)), names_loaded)

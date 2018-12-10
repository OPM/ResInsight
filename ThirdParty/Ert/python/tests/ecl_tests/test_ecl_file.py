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
import shutil
import datetime
import os.path
import gc
from unittest import skipIf

from ecl import EclFileFlagEnum, EclDataType, EclFileEnum
from ecl.eclfile import EclFile, FortIO, EclKW , openFortIO , openEclFile
from ecl.util.util import CWDContext
from ecl.util.test import TestAreaContext, PathContext
from tests import EclTest

def createFile( name , kw_list ):
    with openFortIO(name , mode = FortIO.WRITE_MODE) as f:
        for kw in kw_list:
            kw.fwrite( f )


def loadKeywords( name ):
    kw_list = []
    f = EclFile( name )
    for kw in f:
        kw_list.append( kw )

    return kw_list




class EclFileTest(EclTest):

    def assertFileType(self , filename , expected):
        file_type , step , fmt_file = EclFile.getFileType(filename)
        self.assertEqual( file_type , expected[0] )
        self.assertEqual( fmt_file , expected[1] )
        self.assertEqual( step , expected[2] )


    def test_file_type(self):
        self.assertFileType( "ECLIPSE.UNRST" , (EclFileEnum.ECL_UNIFIED_RESTART_FILE , False , None))
        self.assertFileType( "ECLIPSE.X0030" , (EclFileEnum.ECL_RESTART_FILE , False , 30 ))
        self.assertFileType( "ECLIPSE.DATA"  , (EclFileEnum.ECL_DATA_FILE , None , None ))
        self.assertFileType( "ECLIPSE.FINIT" , (EclFileEnum.ECL_INIT_FILE , True , None ))
        self.assertFileType( "ECLIPSE.A0010" , (EclFileEnum.ECL_SUMMARY_FILE , True , 10 ))
        self.assertFileType( "ECLIPSE.EGRID" , (EclFileEnum.ECL_EGRID_FILE , False  , None ))


    def test_IOError(self):
        with self.assertRaises(IOError):
            EclFile("No/Does/not/exist")


    def test_context( self ):
        with TestAreaContext("python/ecl_file/context"):
            kw1 = EclKW( "KW1" , 100 , EclDataType.ECL_INT)
            kw2 = EclKW( "KW2" , 100 , EclDataType.ECL_INT)
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                kw1.fwrite( f )
                kw2.fwrite( f )

            with openEclFile("TEST") as ecl_file:
                self.assertEqual( len(ecl_file) , 2 )
                self.assertTrue( ecl_file.has_kw("KW1"))
                self.assertTrue( ecl_file.has_kw("KW2"))
                self.assertEqual(ecl_file[1], ecl_file[-1])

    def test_ecl_index(self):
        with TestAreaContext("python/ecl_file/context"):
            kw1 = EclKW( "KW1" , 100 , EclDataType.ECL_INT)
            kw2 = EclKW( "KW2" , 100 , EclDataType.ECL_FLOAT)
            kw3 = EclKW( "KW3" , 100 , EclDataType.ECL_CHAR)
            kw4 = EclKW( "KW4" , 100 , EclDataType.ECL_STRING(23))
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                kw1.fwrite( f )
                kw2.fwrite( f )
                kw3.fwrite( f )
                kw4.fwrite( f )

            ecl_file = EclFile("TEST")
            ecl_file.write_index("INDEX_FILE")
            ecl_file.close()

            ecl_file_index = EclFile("TEST", 0, "INDEX_FILE")
            for kw in ["KW1","KW2","KW3","KW4"]:
                self.assertIn( kw , ecl_file_index )

            with self.assertRaises(IOError):
                ecl_file.write_index("does-not-exist/INDEX")

            os.mkdir("read-only")
            os.chmod("read-only", 0o444)

            with self.assertRaises(IOError):
                ecl_file.write_index("read-only/INDEX")

            with self.assertRaises(IOError):
                ecl_file_index = EclFile("TEST", 0, "index_does_not_exist")

            shutil.copyfile( "INDEX_FILE" , "INDEX_perm_denied")
            os.chmod("INDEX_perm_denied", 0o000)
            with self.assertRaises(IOError):
                ecl_file_index = EclFile("TEST", 0, "INDEX_perm_denied")


            os.mkdir("path")
            shutil.copyfile("TEST" , "path/TEST")
            ecl_file = EclFile("path/TEST")
            ecl_file.write_index("path/index")

            with CWDContext("path"):
                ecl_file = EclFile("TEST" , 0 , "index")


    def test_save_kw(self):
        with TestAreaContext("python/ecl_file/save_kw"):
            data = range(1000)
            kw = EclKW("MY_KEY",  len(data), EclDataType.ECL_INT)
            for index, val in enumerate(data):
                kw[index] = val

            clean_dump = "my_clean_file"
            fortio = FortIO(clean_dump, FortIO.WRITE_MODE)
            kw.fwrite(fortio)
            fortio.close()

            test_file = "my_dump_file"
            fortio = FortIO(test_file, FortIO.WRITE_MODE)
            kw.fwrite(fortio)
            fortio.close()

            self.assertFilesAreEqual(clean_dump, test_file)

            ecl_file = EclFile(test_file, flags=EclFileFlagEnum.ECL_FILE_WRITABLE)
            loaded_kw = ecl_file["MY_KEY"][0]
            self.assertTrue(kw.equal(loaded_kw))

            ecl_file.save_kw(loaded_kw)
            ecl_file.close()

            self.assertFilesAreEqual(clean_dump, test_file)

            ecl_file = EclFile(test_file)
            loaded_kw = ecl_file["MY_KEY"][0]
            self.assertTrue(kw.equal(loaded_kw))

    def test_gc(self):
        kw1 = EclKW("KW1" , 100 , EclDataType.ECL_INT)
        kw2 = EclKW("KW2" , 100 , EclDataType.ECL_INT)
        kw3 = EclKW("KW3" , 100 , EclDataType.ECL_INT)

        for i in range(len(kw1)):
            kw1[i] = i
            kw2[i] = 2*i
            kw3[i] = 3*i

        kw_list = [kw1 , kw2 , kw2]

        with TestAreaContext("context") as ta:
            createFile("TEST" , kw_list )
            gc.collect()
            kw_list2 = loadKeywords( "TEST" )

            for kw1,kw2 in zip(kw_list,kw_list2):
                self.assertEqual( kw1, kw2 )


    def test_broken_file(self):
        with TestAreaContext("test_broken_file"):
            with open("CASE.FINIT", "w") as f:
                f.write("This - is not a ECLISPE file\nsdlcblhcdbjlwhc\naschscbasjhcasc\nascasck c s s aiasic asc")
            f = EclFile("CASE.FINIT")
            self.assertEqual(len(f), 0)


            kw = EclKW("HEADER", 100, EclDataType.ECL_INT)
            with openFortIO("FILE",mode=FortIO.WRITE_MODE) as f:
                kw.fwrite(f)
                kw.fwrite(f)

            with open("FILE", "a+") as f:
                f.write("Seom random gibberish")

            f = EclFile("FILE")
            self.assertEqual(len(f), 2)


            with openFortIO("FILE",mode=FortIO.WRITE_MODE) as f:
                kw.fwrite(f)
                kw.fwrite(f)

            file_size = os.path.getsize("FILE")
            with open("FILE", "a+") as f:
                f.truncate(file_size * 0.75)

            f = EclFile("FILE")
            self.assertEqual(len(f), 1)


    def test_block_view(self):
        with TestAreaContext("python/ecl_file/view"):
            with openFortIO("TEST" , mode = FortIO.WRITE_MODE) as f:
                for i in range(5):
                    header = EclKW("HEADER" , 1 , EclDataType.ECL_INT )
                    header[0] = i

                    data1 = EclKW("DATA1" , 100 , EclDataType.ECL_INT )
                    data1.assign( i )


                    data2 = EclKW("DATA2" , 100 , EclDataType.ECL_INT )
                    data2.assign( i*10 )

                    header.fwrite( f )
                    data1.fwrite( f )
                    data2.fwrite( f )


            ecl_file = EclFile("TEST")
            pfx = 'EclFile('
            self.assertEqual(pfx, repr(ecl_file)[:len(pfx)])
            with self.assertRaises(KeyError):
                ecl_file.blockView("NO" , 1)

            with self.assertRaises(IndexError):
                ecl_file.blockView("HEADER" , 100)

            with self.assertRaises(IndexError):
                ecl_file.blockView("HEADER" , 1000)

            bv = ecl_file.blockView("HEADER" , -1)


            for i in range(5):
                view = ecl_file.blockView("HEADER" , i)
                self.assertEqual( len(view) , 3)
                header = view["HEADER"][0]
                data1 = view["DATA1"][0]
                data2 = view["DATA2"][0]

                self.assertEqual( header[0] , i )
                self.assertEqual( data1[99] , i )
                self.assertEqual( data2[99] , i*10 )


            for i in range(5):
                view = ecl_file.blockView2("HEADER" , "DATA2", i )
                self.assertEqual( len(view) , 2)
                header = view["HEADER"][0]
                data1 = view["DATA1"][0]

                self.assertEqual( header[0] , i )
                self.assertEqual( data1[99] , i )

                self.assertFalse( "DATA2" in view )

            view = ecl_file.blockView2("HEADER" , None, 0 )
            self.assertEqual( len(view) , len(ecl_file))

            view = ecl_file.blockView2(None , "DATA2", 0 )
            #self.assertEqual( len(view) , 2)
            #self.assertTrue( "HEADER" in view )
            #self.assertTrue( "DATA1" in view )
            #self.assertFalse( "DATA2" in view )

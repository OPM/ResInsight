#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'kw_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
from random import randint
from ert.ecl import FortIO, EclTypeEnum, EclKW , openFortIO
from ert.test import ExtendedTestCase, TestAreaContext




class FortIOTest(ExtendedTestCase):
    def setUp(self):
        self.unrst_file = self.createTestPath("Statoil/ECLIPSE/Gurbat/ECLIPSE.UNRST")


    def test_open_read(self):
        f = FortIO(self.unrst_file, FortIO.READ_MODE)
        self.assertIsNotNone(f)


    def test_open_write(self):
        with TestAreaContext("python/fortio/write"):
            f = FortIO("newfile", FortIO.WRITE_MODE)
            self.assertTrue(os.path.exists("newfile"))

    def test_noex(self):
        with self.assertRaises(IOError):
            f = FortIO("odes_not_exist", FortIO.READ_MODE)

    def test_kw(self):
        kw1 = EclKW.create("KW1", 2, EclTypeEnum.ECL_INT_TYPE)
        kw2 = EclKW.create("KW2", 2, EclTypeEnum.ECL_INT_TYPE)

        kw1[0] = 99
        kw1[1] = 77
        kw2[0] = 113
        kw2[1] = 335

        with TestAreaContext("python/fortio/write-kw"):
            f = FortIO("test", FortIO.WRITE_MODE, fmt_file=False)
            kw1.fwrite(f)

            f = FortIO("test", FortIO.APPEND_MODE)
            kw2.fwrite(f)

            f = FortIO("test", fmt_file=False)
            k1 = EclKW.fread(f)
            k2 = EclKW.fread(f)

            self.assertTrue(k1.equal(kw1))
            self.assertTrue(k2.equal(kw2))


    def test_fortio_creation(self):
        with TestAreaContext("python/fortio/create"):
            w = FortIO("test", FortIO.WRITE_MODE)
            rw = FortIO("test", FortIO.READ_AND_WRITE_MODE)
            r = FortIO("test", FortIO.READ_MODE)
            a = FortIO("test", FortIO.APPEND_MODE)

            w.close()
            w.close() # should not fail

    def test_fortio_read_and_write(self):
        with TestAreaContext("python/fortio/read_and_write"):
            f = FortIO("test", FortIO.WRITE_MODE)

            record_size = 4000

            for i, c in enumerate("abcdefghijklmnopqrstuvwxyz"):
                data = bytearray(c * record_size)
                f.writeRecord(data)
                position = f.getPosition()
                self.assertEqual(position, (i + 1) * (record_size + 8))

            f = FortIO("test", FortIO.READ_MODE)

            for c in "abcdefghijklmnopqrstuvwxyz":
                record = f.readRecordAsString(record_size)
                self.assertEqual(record, c * record_size)


    def test_fortio_read_and_write_and_rewrite(self):
        with TestAreaContext("python/fortio/read_and_write_and_rewrite"):
            record_size = 4000

            f = FortIO("complete", FortIO.WRITE_MODE)
            for c in "abcdefghijklmnopqrstuvwxyz":
                data = bytearray(c * record_size)
                f.writeRecord(data)


            f = FortIO("test", FortIO.WRITE_MODE)

            positions = {}
            for c in "abcdefghij-lmnopqrstuvwxyz":
                data = bytearray(c * record_size)
                f.writeRecord(data)
                positions[c] = f.getPosition()


            f = FortIO("test", FortIO.READ_AND_WRITE_MODE)

            f.seek(positions["j"])

            new_data = bytearray("k" * record_size)
            f.writeRecord(new_data)

            f.close()

            self.assertFilesAreEqual("test", "complete")




    def test_context(self):
        with TestAreaContext("python/fortio/context"):
            kw1 = EclKW.create("KW" , 2456 , EclTypeEnum.ECL_FLOAT_TYPE)
            for i in range(len(kw1)):
                kw1[i] = randint(0,1000)

            with openFortIO("file" , mode = FortIO.WRITE_MODE) as f:
                kw1.fwrite( f )

            with openFortIO("file") as f:
                kw2 = EclKW.fread( f )

            self.assertTrue( kw1 == kw2 )
                

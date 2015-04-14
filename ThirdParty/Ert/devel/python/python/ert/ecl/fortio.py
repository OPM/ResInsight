#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'fortio.py' is part of ERT - Ensemble based Reservoir Tool. 
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
"""
Module to support transparent binary IO of Fortran created files.

Fortran is a a funny language; when writing binary blobs of data to
file the Fortran runtime will silently add a header and footer around
the date. The Fortran code:

   integer array(100)
   write(unit) array

it actually writes a head and tail in addition to the actual
data. The header and tail is a 4 byte integer, which value is the
number of bytes in the immediately following record. I.e. what is
actually found on disk after the Fortran code above is:

  | 400 | array ...... | 400 |

The fortio.c file implements the fortio_type C structure which can be
used to read and write these structures transparently. The current
python module is a minimal wrapping of this datastructure; mainly to
support passing of FortIO handles to the underlying C functions. A
more extensive wrapping of the fortio implementation would be easy.
"""
import ctypes
import os
import sys
from ert.cwrap import BaseCClass, CWrapper
from ert.ecl import ECL_LIB


class FortIO(BaseCClass):
    READ_MODE = 1
    WRITE_MODE = 2
    READ_AND_WRITE_MODE = 3
    APPEND_MODE = 4

    def __init__(self, file_name, mode=READ_MODE, fmt_file=False, endian_flip_header=True):

        if mode == FortIO.READ_MODE or mode == FortIO.APPEND_MODE or mode == FortIO.READ_AND_WRITE_MODE:
            if not os.path.exists(file_name):
                raise IOError("File '%s' does not exist!" % file_name)

        if mode == FortIO.READ_MODE:
            c_pointer = FortIO.cNamespace().open_reader(file_name, fmt_file, endian_flip_header)
        elif mode == FortIO.WRITE_MODE:
            c_pointer = FortIO.cNamespace().open_writer(file_name, fmt_file, endian_flip_header)
        elif mode == FortIO.READ_AND_WRITE_MODE:
            c_pointer = FortIO.cNamespace().open_readwrite(file_name, fmt_file, endian_flip_header)
        elif mode == FortIO.APPEND_MODE:
            c_pointer = FortIO.cNamespace().open_append(file_name, fmt_file, endian_flip_header)
        else:
            raise UserWarning("Unknown mode: %d" % mode)

        self.__mode = mode

        super(FortIO, self).__init__(c_pointer)
        self.__open = True

    def close(self):
        if self.__open:
            FortIO.cNamespace().close(self)
            self.__open = False



    def getPosition(self):
        """ @rtype: long """
        return FortIO.cNamespace().get_position(self)

    def seek(self, position):
        # SEEK_SET = 0
        # SEEK_CUR = 1
        # SEEK_END = 2
        FortIO.cNamespace().seek(self, position, 0)

    @classmethod
    def isFortranFile(cls , filename , endian_flip = True):
        """@rtype: bool
        @type filename: str


        Will use heuristics to try to guess if @filename is a binary
        file written in fortran style. ASCII files will return false,
        even if they are structured as ECLIPSE keywords.
        """
        return FortIO.cNamespace().guess_fortran( filename , endian_flip )
        

    def free(self):
        self.close()


class FortIOContextManager(object):

    def __init__(self , fortio):
        self.__fortio = fortio
    
    def __enter__(self):
        return self.__fortio

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__fortio.close()
        return False          


def openFortIO( file_name , mode = FortIO.READ_MODE , fmt_file = False , endian_flip_header = True):
    return FortIOContextManager( FortIO( file_name , mode = mode , fmt_file = fmt_file , endian_flip_header = endian_flip_header ))




cwrapper = CWrapper(ECL_LIB)
cwrapper.registerObjectType("fortio", FortIO)

FortIO.cNamespace().open_reader = cwrapper.prototype("c_void_p fortio_open_reader(char*, bool, bool)")
FortIO.cNamespace().open_writer = cwrapper.prototype("c_void_p fortio_open_writer(char*, bool, bool)")
FortIO.cNamespace().open_readwrite = cwrapper.prototype("c_void_p fortio_open_readwrite(char*, bool, bool)")
FortIO.cNamespace().open_append = cwrapper.prototype("c_void_p fortio_open_append(char*, bool, bool)")

FortIO.cNamespace().write_record = cwrapper.prototype("void fortio_fwrite_record(fortio, char*, int)")

FortIO.cNamespace().get_position = cwrapper.prototype("long fortio_ftell(fortio)")
FortIO.cNamespace().seek = cwrapper.prototype("void fortio_fseek(fortio, int)")

FortIO.cNamespace().close = cwrapper.prototype("bool fortio_fclose(fortio)")
FortIO.cNamespace().guess_fortran = cwrapper.prototype("bool fortio_looks_like_fortran_file(char* , bool)")



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

import libecl
import ctypes
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass

class FortIO(CClass):
    """
    Class to support binary IO of files created by the Fortran runtime.

    The FortIO class is a thin wrapper around the C struct fortio. The
    FortIO class is created to facilitate reading and writing binary
    fortran files. The python implementation (currently) only supports
    instantiation and subsequent use in a C function, but it would be
    simple to wrap the low level read/write functions for Python
    access as well.
    """

    #def __init__(self , filename , mode , fmt_file = False , endian_flip = True):
    #    """
    #    Create a FortIO handle connected to file @filename. @mode as in fopen()
    #
    #    Will create a FortIO handle conntected to the file
    #    @filename. The @mode flag is passed directly to the final
    #    fopen() call and should be "r" to open the file for reading
    #    and "w" for writing.
    #    
    #    The point of the FortIO class is to work with binary files,
    #    but you can set the @fmt_file parameter to True if you want to
    #    read/write formatted ECLIPSE files in restart format. 
    #
    #    By default the FortIO instances will be opened with
    #    endian_flip set to True (this is the correct behaviour for the
    #    ECLIPSE/x86 combination) - but other values are in principle
    #    possible. Observe that the endian flipping only applies to the
    #    header/footer inserted by the Fortran runtime, the actual data
    #    is not touched by the FortIO instance.
    #    """

        
    @classmethod
    def reader(cls , filename , fmt_file = False , endian_flip = True):
        obj = cls( )
        obj.c_ptr = cfunc.fortio_open_reader( filename , fmt_file , endian_flip )
        return obj


    @classmethod
    def writer(cls , filename , fmt_file = False , endian_flip = True):
        obj = cls( )
        obj.c_ptr = cfunc.fortio_open_writer( filename , fmt_file , endian_flip )
        return obj
    

    # Implements normal Python semantics - close on delete.
    def __del__(self):
        """
        Desctructor - will close the filehandle.
        """
        if self.c_ptr:
            cfunc.fortio_close( self )
            
    def close( self ):
        """
        Close the filehandle.
        """
        cfunc.fortio_close( self )
        self.c_ptr = None



# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : fortio <-> FortIO
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType("fortio" , FortIO )


# 3. Installing the c-functions used to manipulate fortio instances.
#    These functions are used when implementing the FortIO class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("fortio")
cfunc.fortio_open_reader = cwrapper.prototype("c_void_p fortio_open_reader(char* , bool , bool)")
cfunc.fortio_open_writer = cwrapper.prototype("c_void_p fortio_open_writer(char* , bool , bool)")
cfunc.fortio_close        = cwrapper.prototype("void     fortio_fclose( fortio )")



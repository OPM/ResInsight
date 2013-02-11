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
from    ert.cwrap.cfile      import CFILE
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


    # The fundamental open functions use a normal Python open() call
    # and pass the filehandle to the C layer, instead of letting the C
    # layer open the file. This way normal Python exception handling
    # will be invoked if there are problems with opening the file.

    @classmethod
    def __wrap(cls , filename, pyfile , fmt_file , endian_flip ):
        obj = cls( )

        obj.pyfile = pyfile
        c_ptr = cfunc.fortio_wrap_FILE( filename , endian_flip , fmt_file , CFILE(obj.pyfile))
        obj.init_cobj( c_ptr , None )
        return obj


    @classmethod
    def open(cls , filename, mode = "r" , fmt_file = False , endian_flip = True):
        """
        Will open of FortIO instance. 
        """
        pyfile = open( filename , mode)
        return cls.__wrap( filename , pyfile , fmt_file , endian_flip )
    
        
    @classmethod
    def reader(cls , filename , fmt_file = False , endian_flip = True):
        """
        Will open a FortIO handle for reading.
        """
        return cls.open( filename , "r" , fmt_file , endian_flip )
    

    @classmethod
    def writer(cls , filename , fmt_file = False , endian_flip = True):
        """
        Will open a FortIO handle for writing.
        """
        return cls.open( filename , "w" , fmt_file , endian_flip )


    # Implements normal Python semantics - close on delete.
    # Because the __del__() operator, i.e. the close(), involves
    # more than just a cfree() function we must override the
    # __del__ operator of the base class.
    def __del__(self):
        """
        Desctructor - will close the filehandle.
        """
        if self.c_ptr:
            self.close( )


    def close( self ):
        """
        Close the filehandle.
        """

        # Seems Python itself allows close() calls on already closed file handles,
        # so we take care to ensure that also the FortIO class supports that.
        if self.pyfile:
            self.pyfile.close()
            cfunc.free_wrapper( self )

        self.pyfile = None
        self.c_ptr = None



# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : fortio <-> FortIO
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType("fortio" , FortIO )


# 3. Installing the c-functions used to manipulate fortio instances.
#    These functions are used when implementing the FortIO class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("fortio")
cfunc.fortio_wrap_FILE    = cwrapper.prototype("c_void_p fortio_alloc_FILE_wrapper( char* , bool , bool , FILE )")
cfunc.free_wrapper        = cwrapper.prototype("void     fortio_free_FILE_wrapper( fortio )")


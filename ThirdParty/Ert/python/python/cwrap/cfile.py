#  Copyright (C) 2016  Statoil ASA, Norway.
#
#  This file is part of ERT - Ensemble based Reservoir Tool.
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

import ctypes
import six
from .prototype import Prototype, PrototypeError
from .basecclass import BaseCClass

class CFILE(BaseCClass):
    """
    Utility class to map a Python file handle <-> FILE* in C
    """
    TYPE_NAME = "FILE"

    _as_file = Prototype(ctypes.pythonapi, "void* PyFile_AsFile(py_object)")

    def __init__(self, py_file):
        """
        Takes a python file handle and looks up the underlying FILE *

        The purpose of the CFILE class is to be able to use python
        file handles when calling C functions which expect a FILE
        pointer. A CFILE instance should be created based on the
        Python file handle, and that should be passed to the function
        expecting a FILE pointer.

        The implementation is based on the ctypes object
        pythonapi which is ctypes wrapping of the CPython api.

          C-function:
             void fprintf_hello(FILE * stream , const char * msg);

          Python wrapper:
             lib = clib.load( "lib.so" )
             fprintf_hello = Prototype(lib, "void fprintf_hello( FILE , char* )")

          Python use:
             py_fileH = open("file.txt" , "w")
             fprintf_hello( CFILE( py_fileH ) , "Message ...")
             py_fileH.close()

        If the supplied argument is not of type py_file the function
        will raise a TypeException.

        Examples: ecl.ecl.ecl_kw.EclKW.fprintf_grdecl()
        """
        c_ptr = self._as_file(py_file)
        try:
            super(CFILE, self).__init__(c_ptr)
        except ValueError as e:
            raise TypeError("Sorry - the supplied argument is not a valid Python file handle!")

        self.py_file = py_file


    def __del__(self):
        pass

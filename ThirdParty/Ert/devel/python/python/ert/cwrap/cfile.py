#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'cfile.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Utility function to map Python filehandle <-> FILE *
"""

import ctypes  
from   cwrap   import *
from   cclass  import CClass

class CFILE(CClass):
    def __init__( self , py_file ):
        """
        Takes a python filehandle and looks up the underlying FILE * 
        
        The purpose of the CFILE class is to be able to use python
        filehandles when calling C functions which expect a FILE
        pointer. A CFILE instance should be created based on the
        Python filehandle, and that should be passed to the function
        expecting a FILE pointer.

        The implementation is based on the ctypes object
        pythonapi which is ctypes wrapping of the CPython api.

          C-function:
             void fprintf_hello(FILE * stream , const char * msg);
            
          Python wrapper:
             lib = clib.load( "lib.so" )
             fprintf_hello = cwrap.prototype("void fprintf_hello( FILE , char* )")
             cwrap.registerType("FILE" , CFILE)
             
          Python use:
             py_fileH = open("file.txt" , "w")
             fprintf_hello( CFILE( py_fileH ) , "Message ...")
             py_fileH.close()

        If the supplied argument is not of type py_file the function
        will raise a TypeException.

        Examples: ert.ecl.ecl_kw.EclKW.fprintf_grdecl()
        """
        self.c_ptr   = cfunc.as_file( py_file ) 
        self.py_file = py_file

        if self.c_ptr is None:
            raise TypeError("Sorry - the supplied argument is not a valid Python filehandle")


    def __del__(self):
        pass



cwrapper = CWrapper( ctypes.pythonapi ) 
cwrapper.registerType( "FILE" , CFILE )
cfunc         = CWrapperNameSpace("FILE")
cfunc.as_file = cwrapper.prototype("c_void_p PyFile_AsFile( py_object )")

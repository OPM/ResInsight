#  Copyright (C) 2012  Statoil ASA, Norway. 
#   
#  The file 'latex.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Module implmenting LaTeX class for wrapping the latex compilation of a file.
"""
import libutil
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass



class LaTeX(CClass):
    def __init__(self , src_file , in_place = False):
        c_ptr = cfunc.alloc( src_file , in_place )
        self.init_cobj( c_ptr , cfunc.free )


    @property
    def runpath(self):
        return cfunc.get_runpath( self )

    def compile(self , ignore_errors = False , with_xref = False):
        return cfunc.compile( self , ignore_errors , with_xref)

    #-----------------------------------------------------------------
    def set_target( self , target_file):
        cfunc.set_target( self , target_file )

    def get_target( self ):
        return cfunc.get_target( self )

    target = property( get_target , set_target)
    #-----------------------------------------------------------------

    #-----------------------------------------------------------------
    def get_timeout( self ):
        return cfunc.get_timeout( self )
    
    def set_timeout( self , timeout):
        cfunc.set_timeout( self , timeout)

    timeout = property( get_timeout , set_timeout)
    #-----------------------------------------------------------------


cwrapper = CWrapper( libutil.lib )
cwrapper.registerType( "latex" , LaTeX )

# 3. Installing the c-functions used to manipulate.
cfunc = CWrapperNameSpace("latex")
cfunc.alloc             = cwrapper.prototype("c_void_p  latex_alloc( char* , bool )")
cfunc.free              = cwrapper.prototype("void      latex_free( latex )")
cfunc.compile           = cwrapper.prototype("bool      latex_compile(latex , bool , bool)")
cfunc.get_runpath       = cwrapper.prototype("char*     latex_get_runpath( latex )") 
cfunc.get_target        = cwrapper.prototype("char*     latex_get_target_file( latex )")
cfunc.set_target        = cwrapper.prototype("void      latex_set_target_file( latex , char* )")
cfunc.set_timeout       = cwrapper.prototype("void      latex_set_timeout( latex , int )")
cfunc.get_timeout       = cwrapper.prototype("int       latex_get_timeout( latex )")

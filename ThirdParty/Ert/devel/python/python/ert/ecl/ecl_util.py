#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_util.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Constants from the header ecl_util.h - some stateless functions.

This module does not contain any class definitions; it mostly consists
of enum definitions/values from ecl_util.h; the enum values are
extracted from the shared library using the
ert.cwrap.cenum.create_enum() function in a semi-automagic manner.

In addition to the enum definitions there are a few stateless
functions from ecl_util.c which are not bound to any class type.
"""

from    ert.cwrap.cwrap       import *
from    ert.cwrap.cenum       import create_enum
from    ert.util.ctime        import ctime 
import  libecl


# ecl_file_enum from ecl_util.h
create_enum( libecl.lib , "ecl_util_file_enum_iget" , "ecl_file_enum" , globals())

# ecl_phase_enum from ecl_util.h
create_enum( libecl.lib , "ecl_util_phase_enum_iget" , "ecl_phase_enum" , name_space = globals())

# ecl_type_enum defintion from ecl_util.h
create_enum( libecl.lib , "ecl_util_type_enum_iget" , "ecl_type_enum" , name_space = globals())



def get_num_cpu( datafile ):
    """
    Parse ECLIPSE datafile and determine how many CPUs are needed.

    Will look for the "PARALLELL" keyword, and then read off the
    number of CPUs required. Will return one if no PARALLELL keyword
    is found.
    """
    return cfunc.get_num_cpu( datafile )


def get_file_type( filename ):
    """
    Will inspect an ECLIPSE filename and return an integer type flag.
    """
    return cfunc.get_file_type( filename , None , None )


def type_name( ecl_type ):
    return cfunc.get_type_name( ecl_type )


def get_start_date( datafile ):
    return cfunc.get_start_date( datafile ).datetime()



cwrapper             = CWrapper( libecl.lib )
cfunc                = CWrapperNameSpace("ecl_util")

cfunc.get_num_cpu    = cwrapper.prototype("int    ecl_util_get_num_cpu( char* )")
cfunc.get_file_type  = cwrapper.prototype("int    ecl_util_get_file_type( char* , bool* , int*)")
cfunc.get_type_name  = cwrapper.prototype("char*  ecl_util_get_type_name( int )")
cfunc.get_start_date = cwrapper.prototype("time_t ecl_util_get_start_date( char* )")

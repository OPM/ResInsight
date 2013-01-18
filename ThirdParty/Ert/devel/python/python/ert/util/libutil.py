#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'libutil.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import ert.cwrap.clib as clib

clib.load("libz" , "libz.so.1")

# Depending on the Fortran compiler which has been used to compile
# blas / lapack the there might be an additional dependency on libg2c:

try:
    # First try to load without libg2c
    clib.load("libblas.so" , "libblas.so.3")
    clib.load("liblapack.so")
except:
    # Then try to load with libg2c
    clib.load("libg2c.so.0")
    clib.load("libblas.so" , "libblas.so.3")
    clib.load("liblapack.so")

lib = clib.ert_load("libert_util.so")
    

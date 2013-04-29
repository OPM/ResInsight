#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'libenkf.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import ert.util.libutil               
import ert.ecl.libecl
import ert.job_queue.libjob_queue
import ert.rms.librms
import ert.cwrap.clib as clib

clib.ert_load("libsched.so")
clib.ert_load("libanalysis.so")
lib = clib.ert_load("libenkf.so")
    

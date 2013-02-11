#!/usr/bin/env python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'job_test.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import time
import sys
import shutil
import os.path
import ert.job_queue.driver as driver
import ert.ecl.ecl as ecl
import socket

default = ecl.ecl_default.default

server_list = { "be" : "lsf-be.no",
                "st" : "lsf-st.no",
                "tr" : "lsf-tr.no" }

src_files     = ["data/eclipse/case/ECLIPSE.DATA" , "data/eclipse/case/include"]
run_path_fmt  = "tmp/simulations/run%d"

default_driver_string = "LOCAL"
num_jobs              = 10
max_running           = 2


def copy_case( target_path , src_files):
    if not os.path.exists( target_path ):
        os.makedirs( target_path )

    print "Creating simulation directory:%s" % target_path
    for file in src_files:
        if os.path.isfile( file ):
            shutil.copy( file , target_path )
        elif os.path.isdir( file ):
            (path , base) = os.path.split( file )
            if not os.path.exists( "%s/%s" % (target_path , base)):
                shutil.copytree( file , "%s/%s" % (target_path , base) )
        else:
            sys.exit("Error")
            


def get_lsf_server():
    host = socket.gethostname()
    site = host[:2]
    lsf_server = server_list.get( site , False)
    if not lsf_server:
        print "Sorry - don't know what is the LSF server in:%s" %  site
        sys.exit()
    return lsf_server

#################################################################

#lsf_driver   = driver.LSFDriver( 1 )
#local_driver = driver.LocalDriver( 3 )
#rsh_driver   = driver.RSHDriver( 1 , [("be-lx655082" , 2)])


queue = ecl.EclQueue( driver_type = driver.LSF_DRIVER , max_running = 3)
joblist = []
case_list = []    
for case_nr in range( num_jobs ):
    copy_case( run_path_fmt % case_nr , src_files )
    case = ecl.EclCase( run_path_fmt % case_nr + "/ECLIPSE.DATA" )
    joblist.append(queue.submit( case.datafile ))

while 1:
    print "Waiting:%02d  Running:%02d   Complete:%02d" % (queue.num_waiting , queue.num_running , queue.num_complete),
    for job in joblist:
        print "[%02d] " % job.status,
    print
    time.sleep( 1 )


queue.block_waiting()

#while queue.running:                      
 #   print "Still running"
 #   time.sleep( 3 )




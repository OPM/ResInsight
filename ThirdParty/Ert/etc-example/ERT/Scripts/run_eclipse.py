#!/usr/bin/python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'run_eclipse.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import sys
import socket
#################################################################
# 
# This is a small script used to run ECLIPSE simulations from ERT. The
# script will set up some environment variables, initialize file
# descriptors and then exec() to the ECLIPSE executable (or to mpirun in
# the case of parallell simulations).
# 
# The script expects three commandline arguments:
# 
#     run_eclipse.py  version  eclipse_name   <num_cpu>
# 
# The @version argument is one of the keys in the the dictionary
# version_table, below. @num_cpu is optional, it will default to one if
# not set.
# 
# 
#################################################################


# The first element in the tuple is the single CPU version to use, and
# the second element is the MPI version to use.

version_table = {"2007.1"     : ("/PATH/ECLIPSE/2007.1/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2007.1/bin/linux_x86_64/eclipse_scampi.exe"),
                 "2007.2"     : ("/PATH/ECLIPSE/2007.2/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2007.2/bin/linux_x86_64/eclipse_scampi.exe"),
                 "2008.1"     : ("/PATH/ECLIPSE/2008.1/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2008.1/bin/linux_x86_64/eclipse_scampi.exe"),
                 "2008.2"     : ("/PATH/ECLIPSE/2008.2/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2008.2/bin/linux_x86_64/eclipse_scampi.exe"),
                 "2009.1"     : ("/PATH/ECLIPSE/2009.1/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2009.1/bin/linux_x86_64/eclipse_scampi.exe"),
                 "2009.2"     : ("/PATH/ECLIPSE/2009.2/bin/linux_x86_64/eclipse.exe"     , "/PATH/ECLIPSE/2009.2/bin/linux_x86_64/eclipse_scampi.exe")}

scali_path     = "/opt/scali"
ecldir         = "/PATH/ECLIPSE"

mpirun         = "%s/bin/mpirun" % scali_path
config_file    = "%s/macros/CONFIG.ECL" % ecldir

config_link    = "ECL.CFG"
env_variables = {"F_UFMTENDIAN"    : "big",
                 "LM_LICENSE_FILE" : "flexlm.server.com" }

max_cpu_sec           = 10000000
max_wall_sec          = 99999999

stdin_file  = "eclipse.stdin"
stdout_file = "eclipse.stdout"
stderr_file = "eclipse.stderr"

# End of configuration options  
#################################################################


# Will take a version string as input, and return the path to the
# executable file. If num_cpu == 1 it will return the serial version,
# whereas the Scali MPI version will be used if num_cpu > 1.
# 
# If the @version input argument can not be found in the version_table
# dictionary, or the corresponding file does not exist, the function
# will fail with a fatal error.
def get_executable( version , num_cpu ):
    t = version_table.get( version , None )
    if t:
        if num_cpu == 1:
            executable = t[0]
        else:
            executable = t[1]
    else:
        fatal_error("Eclipse version:\'%s\' not recognized. Available versions:%s" % (version , version_table.keys()))

    if not os.path.exists( executable ):
        fatal_error("The executable:%s could not be found" % executable)

    return executable




def init_mpi( base_name , num_cpu ):
    # Before the SCALI MPI executable can run we must update the path by prepending
    # ECL_SCALI_LOC/bin and ECL_SCALI_LOC/lib64 to the $PATH and $LD_LIBRARY_PATH
    # environment variables.

    env_variables["PATH"]            = "%s/bin:%s" % ( scali_path , os.getenv("PATH"))
    env_variables["LD_LIBRARY_PATH"] = "%s/lib64:%s" % ( scali_path , os.getenv("LD_LIBRARY_PATH"))
    machine_list = []


    # If the environment variable LSB_HOSTS is set we assume the job is
    # running on LSF - otherwise we assume it is running on the current host.
    #
    # If the LSB_HOSTS variable is indeed set it will be a string like this:
    #
    #       host1  host1  host2  host3
    #
    # i.e. each processs is listed with one hostname entry (i.e. NOT
    # the :num_proc syntax which is used in LSB_MCPU_HOSTS variable.

    machine_file = "%s.mpi" % base_name
    fileH = open( machine_file , "w")

    LSB_HOSTS = os.getenv("LSB_HOSTS")
    if LSB_HOSTS:
        for host in LSB_HOSTS.split():
            fileH.write("%s\n" % host)
    else:
        localhost = socket.gethostname()
        for i in (range(num_cpu)):
            fileH.write("%s\n" % localhost)

    fileH.close()
    return machine_file
    
    

def fatal_error( msg ):
    fileH = open( stderr_file , "w")
    fileH.write( msg )
    fileH.close()
    sys.exit()
    


def init_fd( base_name ):
    # Creating a stupid input file which is connected to stdin
    fileH = open(stdin_file , "w")
    fileH.write("%s\n"   % base_name);
    fileH.write("%s\n"   % base_name);
    fileH.write("%d\n"   % max_cpu_sec);
    fileH.write("%d\n\n" % max_wall_sec);
    fileH.close()

    # Redirecting stdin / stdout / stderr
    fd_stdin  = os.open(stdin_file  , os.O_RDONLY , 0644)
    fd_stdout = os.open(stdout_file , os.O_WRONLY | os.O_TRUNC | os.O_CREAT , 0644);
    fd_stderr = os.open(stderr_file , os.O_WRONLY | os.O_TRUNC | os.O_CREAT , 0644);
    
    os.dup2(fd_stdin  , 0)
    os.dup2(fd_stdout , 1)
    os.dup2(fd_stderr , 2)

    os.close(fd_stdin)
    os.close(fd_stdout)
    os.close(fd_stderr)



def init_path( base_name ):
    smspec_file  = "%s.SMSPEC" % base_name
    fsmspec_file = "%s.FSMSPEC" % base_name

    if os.path.exists( smspec_file ):
        os.unlink( smspec_file );

    if os.path.exists( fsmspec_file ):
        os.unlink( fsmspec_file );

    if not os.path.exists( config_link ):
        os.symlink( config_file , config_link )


def exec_single(executable , env_variables):
    os.execve(executable , [ executable ] , env_variables)


def exec_mpi(executable , base_name , num_cpu , env_variables):
    machine_file = init_mpi( base_name , num_cpu )
    os.execve( mpirun , [mpirun , "-np" , "%s" % num_cpu , "-machinefile" , machine_file , executable , base_name ] , env_variables)


#################################################################
# Main program starts.

if len(sys.argv) < 3 or len(sys.argv) > 4:
    fatal_error("The run_eclipse script needs two/three arguments: eclipse_version   eclipse_base   [num_cpu]")


version            = sys.argv[1]
(run_path , file)  = os.path.split( sys.argv[2] )
(base_name , ext ) = os.path.splitext( file )
if run_path:
    try:
        os.chdir( run_path )
    except:
        fatal_error("The run_eclipse script could not change to directory:%s" % run_path)

if len(sys.argv) == 4:

    # Must support older (svn version before ~ 2844) versions of ERT
    # which do not provide a value for the the <num_cpu> argument, so
    # in this case the run_eclipse.py script will just get the string
    # "<num_cpu>", we catch the ValueError when converting to int, and
    # use the default value num_cpu = 1.
    
    try:
        num_cpu = int( sys.argv[3] )
    except ValueError:
        num_cpu = 1
else:
    num_cpu = 1

executable = get_executable(version , num_cpu)
init_path( base_name )
init_fd( base_name )
if num_cpu == 1:
    exec_single( executable , env_variables)
else:
    exec_mpi( executable , base_name , num_cpu , env_variables )

    

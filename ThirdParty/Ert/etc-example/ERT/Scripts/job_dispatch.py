#!/usr/bin/python
#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'job_dispatch.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import sys
import os
import os.path
import socket
import time
import random


OK_file     = "OK"
EXIT_file   = "EXIT"
STATUS_file = "STATUS"
run_path    = sys.argv[1]
sleep_time  = 10           # Time to sleep before exiting the script - to let the disks sync up. 
short_sleep =  2

#################################################################

def redirect(file , fd , open_mode):
    new_fd = os.open(file , open_mode , 0644)   
    os.dup2(new_fd , fd)
    os.close(new_fd)


def cond_symlink(target , src):
    if not os.path.exists(src):
        os.symlink( target , src )


def cond_unlink(file):
    if os.path.exists(file):
        os.unlink(file)



def exec_job(job , executable):
    if job["stdin"]:
        redirect(job["stdin"]  , 0 , os.O_RDONLY)

    if job["stdout"]:
        redirect(job["stdout"] , 1 , os.O_WRONLY | os.O_TRUNC | os.O_CREAT )

    if job["stderr"]:
        redirect(job["stderr"] , 2 , os.O_WRONLY | os.O_TRUNC | os.O_CREAT )

    if job["environment"]:
        env = job["environment"]
        for key in env.keys():
            os.putenv(key , env[key])
    os.execvp(executable , [executable] + job["argList"])



def unlink_empty(file):
    if os.path.exists(file):
        st = os.stat( file )
        if st.st_size == 0:
            os.unlink( file )



def cleanup( job ):
    if job["stdout"]:
        unlink_empty( job["stdout"] )
    if job["stderr"]:
        unlink_empty( job["stderr"] )

    if job["license_link"]:
        os.unlink(job["license_link"])



# This function implements a simple "personal license" system limiting
# how many instances of this job can run concurrently. Observe that the
# limiting is based on pr. invocation of the queue system (i.e. ERT
# binary) and pr user. The system works as follows:
#
#  1. The job is initilized with a license_path and a max_running
#     variable.
#
#  2. In the licens_path directory a license file is made.
#
#  3. For each instance a random hard-link is created to the license
#     file - this is how the number of concurrent uses is counted.
#
#  4. When the external program is finished the hard link is removed.


def license_check( job ):
    job["license_link"] = None
    if job.has_key("max_running"):
        if job["max_running"]:
            job["license_file"] = "%s/%s" % (job["license_path"] , job["name"])
            max_running         = job["max_running"]
            license_file        = job["license_file"]
            while True:
                job["license_link"] = "%s/%d" % (job["license_path"] , random.randint(100000,999999))
                if not os.path.exists(job["license_link"]):
                    break
                

            if not os.path.exists(license_file):
                fileH = open(license_file , "w")
                fileH.write("This is a license file for job:%s" % job["name"])
                fileH.close()
                
                
            stat_info = os.stat(license_file)
            currently_running = stat_info[3] - 1

            while True:
                stat_info = os.stat(license_file)
                currently_running = stat_info[3] - 1
                if currently_running < max_running:
                    break
                else:
                    time.sleep(5)

            os.link(license_file , job["license_link"])

            while True:
                stat_info = os.stat(license_file) 
                currently_running = stat_info[3] - 1
                if currently_running <= max_running:
                    break # OK - now we can leave the building - and let the job start
                else:
                    time.sleep(5)


# Compatibility mode which must be retained until all ert prior to
# svn 2709 has been removed.

def get_executable( job ):
    executable = job.get("executable")
    if not executable:
        executable = job.get("portable_exe")
        if not executable:
            cpu = os.uname()[4]
            if job["platform_exe"].has_key(cpu):
                executable = job["platform_exe"][cpu]
            else:
                return (False, 0 , "%s : did not recognize platform:%s" % (job["name"] , cpu))  

    return executable


    
def run_one(job):
    license_check( job )
    if job["stdin"]:
        if not os.path.exists(job["stdin"]):
            return (False , 0 , "Could not locate stdin file: %s" % job["stdin"])
        
    if job["start_file"]:
        if not os.path.exists(job["start_file"]):
            return (False , -1 , "Could not locate start_file:%s" % job["start_file"])

    
    
    executable = get_executable( job )
    start_time = time.time()  
    pid = os.fork()
    if pid == 0: 
        exec_job(job  , executable)
    else:
        if job["max_running_minutes"]:
            proc_path = "/proc/%d" % pid
            # There is max run time on the job.
            while True:
                time.sleep( short_sleep )     
                if os.path.exists( proc_path ):    #  Checking if the 
                    run_time = time.time() - start_time
                    if run_time > (60 * job["max_running_minutes"]):
                        return (False , 0 , "Run time of %d minutes exceeded for job:%s" % ( job["max_running_minutes"] , job["name"]))
                else:
                    break                     # The job is no longer running
                
        else:
            (return_pid , exit_status) = os.waitpid(pid , 0)


        if job["target_file"]:
            if os.path.exists(job["target_file"]):
                stat = os.stat(job["target_file"])
                if stat.st_ctime > start_time:
                    cleanup(job)
                    return (True , 0 , "")
                else:
                    cleanup(job)
                    return (True , 0 , "Hmmm - seems the target file has not been updated - let the job suceed anyway...??")
            else:
                cleanup(job)
                return (False , exit_status , "%s : could not find target_file:%s" % (job["name"] , job["target_file"]))
        else:
            cleanup(job)
            return (True , exit_status , "Target file not speced") # Do not really look at exit status yet...



#################################################################

#################################################################

os.nice(19)    
if not os.path.exists( run_path ):
    sys.stderr.write("*****************************************************************\n");
    sys.stderr.write("** FATAL Error: Could not find dirctory: %s \n" % run_path)
    sys.stderr.write("** CWD: %s\n" % os.getcwd())
    sys.stderr.write("*****************************************************************\n");

    fileH = open(EXIT_file , "w") 
    fileH.write("Could not locate:%s " % run_path)
    fileH.write("CWD: %s" % os.getcwd())
    fileH.close()
    sys.exit(-1)

os.chdir( run_path )
cond_unlink(EXIT_file)
cond_unlink(STATUS_file)
cond_unlink(OK_file)
fileH = open(STATUS_file , "a")
fileH.write("%-40s: %s/%s\n" % ("Current host:" , socket.gethostname() , os.uname()[4]))
fileH.close()

sys.path.append( os.getcwd() )
import jobs
random.seed()

for job in jobs.jobList:
    # To ensure compatibility with old versions.
    if not job.has_key("max_running_minutes"):
        job["max_running_minutes"] = None

if len(sys.argv) == 2:
    # Normal batch run.
    for job in jobs.jobList:
        fileH = open(STATUS_file , "a")
        now = time.localtime()
        fileH.write("%-32s: %02d:%02d:%02d .... " % (job["name"] , now.tm_hour , now.tm_min , now.tm_sec))
        fileH.close()
        (OK , exit_status, error_msg) = run_one(job)
        now = time.localtime()
        if OK:
            fileH = open(STATUS_file , "a")
            fileH.write("%02d:%02d:%02d \n" % (now.tm_hour , now.tm_min , now.tm_sec))
            fileH.close()
        else:
            fileH = open(EXIT_file , "w") 
            fileH.write("%02d:%02d:%02d \n" % (now.tm_hour , now.tm_min , now.tm_sec))
            fileH.write("%s : failed\n" % job["name"])
            fileH.write("%s\n" % error_msg) 
            fileH.close()
            sys.exit(exit_status)
        

    if OK:
        fileH = open("OK" , "w")
        fileH.write("All jobs complete") 
        fileH.close()
        time.sleep( sleep_time )   # Let the disks sync up 
else:
    #Interactive run
    jobHash = {}
    for job in jobs.jobList:
        jobHash[job["name"]] = job

    for job_name in sys.argv[2:]:
        if jobHash.has_key( job_name ):
            job = jobHash[job_name]
            print "Running job: %s ... " % job_name,
            sys.stdout.flush()
            (OK , exit_status, error_msg) = run_one( job )
            if OK:
                print "OK"
            else:
                print "failed ...."
                print "-----------------------------------------------------------------"
                if job["stderr"]:
                    print "Error:%s " % error_msg
                    if os.path.exists(job["stderr"]):
                        fileH = open(job["stderr"],"r")
                        for line in fileH.readlines():
                            print line,
                        fileH.close()
                print "-----------------------------------------------------------------"
                sys.exit()
        else:
            print "Job: %s does not exist. Available jobs:" % job_name
            for j in jobs.jobList:
                print "   %s" % j["name"]
    

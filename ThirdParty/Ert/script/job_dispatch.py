#!/usr/bin/env python
from __future__ import print_function
import sys
import os
import os.path
import socket
import time
import random
import subprocess


OK_file       =  "OK"
EXIT_file     =  "EXIT"
STATUS_file   =  "STATUS"
run_path      =  sys.argv[1]
sleep_time    =  10           # Time to sleep before exiting the script - to let the disks sync up. 
short_sleep   =  2

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
    if job.get("stdin"):
        redirect(job["stdin"]  , 0 , os.O_RDONLY)

    if job.get("stdout"):
        redirect(job["stdout"] , 1 , os.O_WRONLY | os.O_TRUNC | os.O_CREAT )

    if job.get("stderr"):
        redirect(job["stderr"] , 2 , os.O_WRONLY | os.O_TRUNC | os.O_CREAT )

    if job.get("environment"):
        env = job["environment"]
        for key in env.keys():
            os.putenv(key , env[key])
    argList = [ executable ]
    if job.get("argList"):
        argList += job["argList"]
    os.execvp(executable , argList )


def job_process(job , executable):
    import subprocess
    argList = [ executable ]
    if job.get("argList"):
        argList += job["argList"]
    
    P = subprocess.Popen( argList , 
                          stdin  = job.get("stdin"),
                          stdout = job.get("stdout"),
                          stderr = job.get("stderr"),
                          env = job.get("environment") )
    return P
                          

def kill_process( job , P ):
    fileH = open( EXIT_file , "a")
    fileH.write("Job:%s has been running for more than %d minutes - explicitly killed.\n" % (job["name"] , job["max_running_minutes"]))
    fileH.close()

    try:
        P.kill()    # P.kill became available in Python2.6
    except AttributeError:
        os.system("kill -9 %s" % P.pid)


def unlink_empty(file):
    if os.path.exists(file):
        st = os.stat( file )
        if st.st_size == 0:
            os.unlink( file )



def cleanup( job ):
    if job.get("stdout"):
        unlink_empty( job["stdout"] )
    if job.get("stderr"):
        unlink_empty( job["stderr"] )
    if job.get("license_link"):
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
    if job.get("stdin"):
        if not os.path.exists(job["stdin"]):
            return (False , 0 , "Could not locate stdin file: %s" % job["stdin"])
        
    if job.get("start_file"):
        if not os.path.exists(job["start_file"]):
            return (False , -1 , "Could not locate start_file:%s" % job["start_file"])

    if job.get("error_file"):
        if os.path.exists( job.get("error_file")):
            os.unlink( job.get("error_file") )
    

    executable = get_executable( job )
    start_time = time.time()  

    if job.get("max_running_minutes"):
        P = job_process( job , executable)
        while True:
            time.sleep( short_sleep )   
            run_time = time.time() - start_time
            poll = P.poll()
            if poll is None:
                # Still running
                if run_time > job["max_running_minutes"] * 60:
                    # Have been running to long - kill it
                    kill_process( job , P )
                    break
            else:
                # Have completed within the time limits
                break
        exit_status = 0  # NOOOT properly used
    else:
        pid = os.fork()
        if pid == 0: 
            exec_job(job  , executable)
        else:
            (return_pid , exit_status) = os.waitpid(pid , 0)


    # Check success of job; look for both target_file and
    # error_file. Both can be used to signal failure
    # independently.

    if job.get("target_file"):
        if os.path.exists(job["target_file"]):
            stat = os.stat(job["target_file"])
            if stat.st_ctime > start_time:
                status = (True , 0 , "")
            else:
                status = (True , 0 , "Hmmm - seems the target file has not been updated - let the job suceed anyway...??")
        else:
            status = (False , exit_status , "%s : could not find target_file:%s" % (job["name"] , job["target_file"]))
    else:
        status = (True , exit_status , "Target file not specified") # Do not really look at exit status yet...

    if status[0]:
        if job.get("error_file"):
            if os.path.exists( job.get("error_file") ):
                status = (False , -1 , "Found the error file:%s - job failed" % job.get("error_file"))

    return status
                
                



#################################################################

#################################################################

os.nice(19)    
if not os.path.exists( run_path ):
    sys.stderr.write("*****************************************************************\n");
    sys.stderr.write("** FATAL Error: Could not find directory: %s \n" % run_path)
    sys.stderr.write("** CWD: %s\n" % os.getcwd())
    sys.stderr.write("*****************************************************************\n");

    fileH = open(EXIT_file , "w") 
    fileH.write("Could not locate:%s " % run_path)
    fileH.write("CWD: %s" % os.getcwd())
    fileH.close()
    sys.exit(-1)


#################################################################
# 1. Change current directory to the runpath. 
# 2. Modify the sys.path variable to include the runpath
# 3. Import the jobs module.
#################################################################
os.chdir( run_path )
sys.path.append( os.getcwd() )
import jobs

# The jobs module can optionally have a dictionary 'options', which can be
# used to modify some of the global run time properties of the script.
if hasattr(jobs , "options"):
    options     = getattr( jobs , "options")
    OK_file     = options.get("OK_file"     , OK_file )
    EXIT_file   = options.get("EXIT_file"   , EXIT_file )
    STATUS_file = options.get("STATUS_file" , STATUS_file )
    sleep_time  = options.get("sleep_time"  , sleep_time )
    
cond_unlink(EXIT_file)
cond_unlink(STATUS_file)
cond_unlink(OK_file)
fileH = open(STATUS_file , "a")
fileH.write("%-32s: %s/%s\n" % ("Current host" , socket.gethostname() , os.uname()[4]))
fileH.close()


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
            fileH = open(EXIT_file , "a") 
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
        # This is totally unpredictable if there more jobs with
        # the same name.
        if jobHash.has_key( job_name ):
            job = jobHash[job_name]
            print('Running job: %s ... ' % job_name, end='')
            sys.stdout.flush()
            (OK , exit_status, error_msg) = run_one( job )
            if OK:
                print('OK')
            else:
                print('failed ...')
                print('-----------------------------------------------------------------')
                if job.get("stderr"):
                    print('Error:%s ' % error_msg)
                    if os.path.exists(job["stderr"]):
                        fileH = open(job["stderr"],"r")
                        for line in fileH.readlines():
                            print(line, end='')
                        fileH.close()
                print('-----------------------------------------------------------------')
                sys.exit()
        else:
            print('Job: %s does not exist. Available jobs:' % job_name)
            for j in jobs.jobList:
                print('   %s' % j['name'])

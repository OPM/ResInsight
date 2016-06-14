#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'ecl_queue.py' is part of ERT - Ensemble based Reservoir Tool. 
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
Implements a queue to run many ECLIPSE simulations.

The EclQueue class should be used when you have more ECLIPSE
simulations than you can perform concurrently; the queue will start
simulations when there is available computing resources. The EclQueue
can be combined with LSF, in that case the EclQueue will limit the
number of jobs submitted to LSF, but it is clearly most relevant when
simulating locally or on workstations through rsh commands.

The queue is based on the use of 'driver' to communicate with the low
level systems for running the simulations. The driver must be
instantiated before you can create a queue. The driver is implemented
in the driver module in ert.job_queue.driver.

The EclQueue class is a decendant of the JobQueue class, which is
specialized to only submit ECLIPSE simulations. To fully understand
the EclQueue class it is important to consult the documentation of the
JobQueue class in ert.job_queue.queue as well.
"""
import os.path
from ert.ecl import EclUtil, EclDefault
from ert.job_queue import JobQueue, Driver


class EclQueue(JobQueue):
    def __init__(self,
                 driver=None,
                 driver_type=None,
                 driver_options=None,
                 ecl_version=None,
                 ecl_cmd=None,
                 max_running=0,
                 size=0):

        """
        Create a new queue instance to manage ECLIPSE simulations.

        The constructor will create a new EclQueue instance which can
        be used to manage ECLIPSE simulations. All the ECLIPSE
        simulations managed by this queue instance will be 'of the
        same type', i.e. they will run the same ECLIPSE version using
        the same command to run ECLIPSE. 

        The queue starts running (i.e. managing jobs) as soon as it is
        created. Subsequently the user must add jobs to run using the
        submit() method, or alternatively the submit() method of an
        EclCase instance.
        
        The queue will run in a separate thread, and there is nothing
        preventing your script from finishing (i.e. exit) even though
        the queue thread is still running; in that case the jobs which
        are still waiting in the queue will not be started[1]. To
        protect against this premature exit you must use on of the
        methods:

          - queue.block_waiting()
          - queue.block()
          - while queue.running:
                .....

        All simulations which have been started by the queue instance
        will continue running even if the queue itself goes out of
        scope and the script exits.

        There are many arguments to the constructor, all of them are
        optional with sensible default values. There are essentially
        three groups of arguments:

        Driver related arguments  
        ------------------------
        driver: This should be an instance of Driver (or one of the
           descendants LSFDriver, LocalDriver, or ...) from the
           ert.job_queue.driver module. If the driver is None the
           queue will instanstiate a driver itself.

        driver_type: If the driver is None the queue needs to
           instantiate a private driver. It will then instantiate a
           driver of the type given by driver_type.

        driver_options: If the queue needs to instantiate a private
           driver it will pass the options given by the
           driver_options argument to the driver constructor.
              
        There is some dependance between the driver related arguments;
        if a @driver argument is passed the @driver_type and
        @driver_options arguments are not considered.

        ECLIPSE related arguments
        -------------------------
        ecl_version: The ECLIPSE version used when simulating. This
           should be a string of the type "2009.1".

        ecl_cmd: The path to the executable used to invoke
           ECLIPSE. The executable will be invoked with commandline
           arguments: 
                         version   data_file   num_cpu
           And this executable is responsible for then starting the
           real ECLIPSE binary. 

        Other arguments
        ---------------
        max_running: The maximum number of jobs the queue can run
           concurrently. The default value max_running=0 means that
           the queue can start an unlimited number of jobs; in the
           case of e.g. LSF the default value can be sensible,
           otherwise you should probably set a value.

        size: This is the total number of simulations we want to run
           with the queue; this is further documented in the
           ert.job_queue.queue.JobQueue class.

        Use example
        -----------
        import ert.ecl.ecl as ecl
        import ert.job_queue.driver as driver
        
        queue = ecl.EclQueue( driver_type = driver.LOCAL , max_running = 4 , size = 100)
        data_file_fmt = "/path/to/ECLIPSE/sim%d/ECL.DATA"
        for i in range(100):
            queue.submit( data_file_fmt % i )
        
        queue.block()
        
        [1]: It should be possible with a fork based solution to let
             the queue stay alive and continue managing jobs even
             after the main thread has exited - not yet :-(
        """
        if ecl_cmd is None:
            ecl_cmd = EclDefault.ecl_cmd()

        if driver_type is None:
            driver_type = EclDefault.driver_type()

        if ecl_version is None:
            ecl_version = EclDefault.ecl_version()

        self.ecl_version = ecl_version
        self.ecl_cmd = ecl_cmd
        if driver is None:
            if driver_options is None:
                driver_options = EclDefault.driver_options()[driver_type]
            driver = Driver(driver_type, max_running=max_running, options=driver_options)

        super(EclQueue, self).__init__(driver, size=size)


    def submitDataFile(self, data_file):
        """
        Will submit a new simulation of case given by @data_file.
        """
        (path_base, ext) = os.path.splitext(data_file)
        (run_path, base) = os.path.split(path_base)

        num_cpu = EclUtil.get_num_cpu(data_file)
        argv = [self.ecl_version, path_base, "%s" % num_cpu]

        return self.submit(self.ecl_cmd, run_path, base, argv, num_cpu=num_cpu)


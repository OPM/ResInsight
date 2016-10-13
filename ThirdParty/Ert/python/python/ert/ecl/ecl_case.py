#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'ecl_case.py' is part of ERT - Ensemble based Reservoir Tool.
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
Implements EclCase class which is a container for one ECLIPSE case.

This module is a pure Python module which does not directly invoke any
C based functions.
"""
import os
import warnings
from ert.ecl import EclRFTFile, EclGrid, EclSum, EclUtil



class EclCase:
    """
    Small container for one ECLIPSE case.

    Mostly a wrapper around an ECLIPSE datafile, along with properties
    to load the corresponding summary, grid and rft files. In addition
    there are methods run() and submit() to run the ECLIPSE
    simulation.
    """
    def __init__(self , input_case):
        """
        Create a new case based on path/basename.

        The @input_case argument should be the basename of the ECLIPSE
        case, it can contain an additional path component. The
        @input_case argument can contain an extension, but that is not
        necessary; it does not need to point an existing file.

        These are all valid:

           case1 = EclCase( "ECLIPSE" )
           case2 = EclCase( "relative/path/ECLIPSE.SMSPEC" )
           case3 = EclCase( "/absolute/path/simulation/ECLIPSE_3.xxx" )

        """
        warnings.warn("The EclCase class is deprecated - instantiate the EclSum / EclGrid / ... classes directly." , DeprecationWarning)

        self.case = input_case
        (path , tmp) = os.path.split( input_case )
        if path:
            self.__path = os.path.abspath( path )
        else:
            self.__path = os.getcwd()
        (self.__base , self.ext) = os.path.splitext( tmp )

        self.__sum         = None
        self.__grid        = None
        self.__data_file   = None
        self.__rft         = None


    @property
    def datafile( self ):
        """
        Will return the full path to the ECLIPSE data file.

        Observe that this method is purely about string manipulation;
        i.e. it is not checked if the datafile actually exists.
        """

        if not self.__data_file:
            self.__data_file = "%s/%s.DATA" % ( self.__path , self.__base )
        return self.__data_file


    @property
    def sum( self ):
        """
        An EclSum instance for the current case; or None.

        Observe that accessing the summary of a running ECLIPSE
        simulation, i.e. to check progress, is extremely error
        prone. During the actual simulation the summary files on disk
        are invalid a large part of the time, and an attempt to load
        at these times will lead to an instant 'crash and burn'.
        """
        if not self.__sum:
            self.__sum = EclSum( self.case )
        return self.__sum


    @property
    def grid( self ):
        """
        An EclGrid instance for the current case; or None.
        """
        if not self.__grid:
            self.__grid = EclGrid( self.case )
        return self.__grid


    @property
    def rft_file( self ):
        """
        An EclRFTFile instance for the current case; or None.
        """
        if not self.__rft:
            self.__rft = EclRFTFile( self.case )
        return self.__rft


    @property
    def base( self ):
        """
        Will return the ECLIPSE basename of the current case.
        """
        return self.__base


    @property
    def path( self ):
        """
        Will return the absolute path of the current case.
        """
        return self.__path


    def run( self,
             ecl_cmd = None,
             ecl_version = None,
             driver = None,
             driver_type = None,
             driver_options = None,
             blocking = False ):
        """
        Will start an ECLIPSE simulation of the current case.

        The method has a long and nasty argument list, but all
        arguments have sensible defaults, and you probably do not need
        to enter any at all. The arguments are as follows:

          ecl_cmd: The command used to run ECLIPSE. This will
             typically be a script of some kind. The command will be
             called with three commandline arguments: version datafile
             num_cpu

          ecl_version: The eclipse version you want to use, this
             should be a string of the type "2010.2".

          driver: This should be an instance of Driver() from
             ert.job_queue.driver. If driver is None the method will
             create a new driver instance.

          driver_type: If the driver is none the method will create a
             new driver instance, it will create a driver of this type
             (i.e. LOCAL, LSF or RSH).

          driver_options: When creating a new driver, these options
             will be used.

          blocking: If blocking is True the method will not return
             before the simulation is complete, otherwise the method
             will return immediately. If blocking is False the ECLIPSE
             simulation will continue even if the python script
             exits.

        Observe that there are some dependencies between the arguments:

          * If both driver and driver_type are present the existing
            driver instance will be used, and the driver_type argument
            will be ignored.

          * The driver_options argument will only be used when
            creating a new driver instance, and will not be used to
            modify an existing driver instance.
        """
        import ert.job_queue.driver as queue_driver

        num_cpu = EclUtil.get_num_cpu( self.datafile )
        argv = [ecl_version , self.datafile , num_cpu]

        job = driver.submit( self.base , ecl_cmd , self.path , argv , blocking = blocking)
        return job

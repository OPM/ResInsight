#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'job.py' is part of ERT - Ensemble based Reservoir Tool. 
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


import time
import datetime
from ert.cwrap.cclass import CClass

# Enum values nicked from libjob_queue/src/basic_queue_driver.h
STATUS_PENDING = 16
STATUS_RUNNING = 32
STATUS_DONE = 64
STATUS_EXIT = 128


class Job(CClass):
    def __init__(self, driver, c_ptr, queue_index, blocking=False):
        self.driver = driver
        self.init_cobj(c_ptr, self.driver.free_job)
        self.submit_time = datetime.datetime.now()
        self.queue_index = queue_index


    def block( self ):
        while True:
            status = self.status()
            if status == STATUS_DONE or status == STATUS_EXIT:
                break
            else:
                time.sleep(1)

    def kill( self ):
        self.driver.kill_job(self)


    @property
    def run_time( self ):
        td = datetime.datetime.now() - self.submit_time
        return td.seconds + td.days * 24 * 3600

    @property
    def status( self ):
        st = self.driver.get_status(self)
        return st

    @property
    def running( self ):
        status = self.driver.get_status(self)
        if status == STATUS_RUNNING:
            return True
        else:
            return False


    @property
    def pending( self ):
        status = self.driver.get_status(self)
        if status == STATUS_PENDING:
            return True
        else:
            return False

    @property
    def complete( self ):
        status = self.driver.get_status(self)
        if status == STATUS_DONE or status == STATUS_EXIT:
            return True
        else:
            return False
        


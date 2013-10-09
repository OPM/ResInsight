#  Copyright (C) 2013  Statoil ASA, Norway.
#
#  The file 'job_status_type_enum.py' is part of ERT - Ensemble based Reservoir Tool.
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
from ert.cwrap import BaseCEnum
from ert.job_queue import JOB_QUEUE_LIB


class JobStatusType(BaseCEnum):
    JOB_QUEUE_NOT_ACTIVE = None     # This value is used in external query routines - for jobs which are (currently) not active. */
    JOB_QUEUE_WAITING = None        # A node which is waiting in the internal queue.
    JOB_QUEUE_SUBMITTED = None      # Internal status: It has has been submitted - the next status update will (should) place it as pending or running.
    JOB_QUEUE_PENDING = None        # A node which is pending - a status returned by the external system. I.e LSF
    JOB_QUEUE_RUNNING = None        # The job is running
    JOB_QUEUE_DONE = None           # The job is done - but we have not yet checked if the target file is produced */
    JOB_QUEUE_EXIT = None           # The job has exited - check attempts to determine if we retry or go to complete_fail   */
    JOB_QUEUE_USER_KILLED = None    # The job has been killed by the user - can restart. */
    JOB_QUEUE_USER_EXIT = None      # The whole job_queue has been exited by the user - the job can NOT be restarted. */
    JOB_QUEUE_SUCCESS = None
    JOB_QUEUE_RUNNING_CALLBACK = None
    JOB_QUEUE_FAILED = None

JobStatusType.populateEnum(JOB_QUEUE_LIB, "queue_driver_status_enum_iget")
JobStatusType.registerEnum(JOB_QUEUE_LIB, "job_status_type_enum")





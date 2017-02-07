#  Copyright (C) 2012  Statoil ASA, Norway.
#
#  The file 'sched_file.py' is part of ERT - Ensemble based Reservoir Tool.
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
import os.path

from cwrap import BaseCClass
from ert.sched import SchedulePrototype
from ert.util import CTime


class SchedFile(BaseCClass):
    TYPE_NAME = "sched_file"

    _parse  = SchedulePrototype("void* sched_file_parse_alloc( char*, time_t )", bind = False)
    _write  = SchedulePrototype("void  sched_file_fprintf_i( sched_file , int , char* , bool)")
    _length = SchedulePrototype("int   sched_file_get_num_restart_files( sched_file )")
    _free   = SchedulePrototype("void  sched_file_free( sched_file )")

    def __init__(self, filename, start_time):
        if os.path.isfile(filename):
            c_ptr = self._parse(filename, CTime(start_time))
            super(SchedFile, self).__init__(c_ptr)
            if not c_ptr:
                err_msg = 'start_time = "%s", filename = "%s"' % (str(start_time), str(filename))
                raise ValueError('Unable to construct SchedFile with %s.' % err_msg)
        else:
            raise IOError('No such file "%s"' % filename)

    @property
    def length(self):
        """ @rtype: int """
        return len(self)

    def __len__(self):
        return self._length()

    def write(self, filename, num_dates, add_end=True):
        self._write( num_dates, filename, add_end )

    def free(self):
        self._free( )

    def __repr__(self):
        return 'SchedFile(len = %d) %s' % (len(self), self._ad_str())

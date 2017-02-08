#  Copyright (C) 2013  Statoil ASA, Norway. 
#   
#  The file 'time_map.py' is part of ERT - Ensemble based Reservoir Tool. 
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
import errno 

from cwrap import BaseCClass
from ert.enkf import EnkfPrototype
from ert.util import CTime


class TimeMap(BaseCClass):
    TYPE_NAME = "time_map"

    _fread_alloc_readonly       = EnkfPrototype("void*  time_map_fread_alloc_readonly(char*)", bind = False)
    _alloc                      = EnkfPrototype("void*  time_map_alloc()", bind = False)
    _load                       = EnkfPrototype("bool   time_map_fread(time_map , char*)")
    _save                       = EnkfPrototype("void   time_map_fwrite(time_map , char*)")
    _fload                      = EnkfPrototype("bool   time_map_fscanf(time_map , char*)")
    _iget_sim_days              = EnkfPrototype("double time_map_iget_sim_days(time_map, int)")
    _iget                       = EnkfPrototype("time_t time_map_iget(time_map, int)")
    _size                       = EnkfPrototype("int    time_map_get_size(time_map)")
    _try_update                 = EnkfPrototype("bool   time_map_try_update(time_map , int , time_t)")
    _is_strict                  = EnkfPrototype("bool   time_map_is_strict( time_map )")
    _set_strict                 = EnkfPrototype("void   time_map_set_strict( time_map , bool)")
    _lookup_time                = EnkfPrototype("int    time_map_lookup_time( time_map , time_t)")
    _lookup_time_with_tolerance = EnkfPrototype("int    time_map_lookup_time_with_tolerance( time_map , time_t , int , int)")
    _lookup_days                = EnkfPrototype("int    time_map_lookup_days( time_map ,         double)")
    _last_step                  = EnkfPrototype("int    time_map_get_last_step( time_map )")
    _upgrade107                 = EnkfPrototype("void   time_map_summary_upgrade107( time_map , ecl_sum )")
    _free                       = EnkfPrototype("void   time_map_free( time_map )")

    def __init__(self, filename = None):
        c_ptr = self._alloc()
        super(TimeMap, self).__init__(c_ptr)
        if filename:
            self.load(filename)


    def load(self, filename):
        if os.path.isfile( filename ):
            self._load(filename)
        else:
            raise IOError(( errno.ENOENT , "File not found: %s" % filename))


    def fwrite(self, filename):
        self._save(filename)


    def fload(self , filename):
        """
        Will load a timemap as a formatted file consisting of a list of dates: DD/MM/YYYY
        """
        if os.path.isfile( filename ):
            OK = self._fload(filename)
            if not OK:
                raise Exception("Error occured when loading timemap from:%s" % filename)
        else:
            raise IOError(( errno.ENOENT , "File not found: %s" % filename))



    def isStrict(self):
        return self._is_strict()


    def setStrict(self , strict):
        return self._set_strict(strict)
        

    def getSimulationDays(self, step):
        """ @rtype: double """
        if not isinstance(step, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if step < 0 or step >= size:
            raise IndexError("Index out of range: 0 <= %d < %d" % (step, size))

        return self._iget_sim_days(step)


    def __getitem__(self, index):
        """ @rtype: CTime """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index < 0 or index >= size:
            raise IndexError("Index out of range: 0 <= %d < %d" % (index, size))

        return self._iget(index)

    def __setitem__(self , index , time):
        self.update( index , time )


    def update(self , index , time):
        if self._try_update(index , CTime(time)):
            return True
        else:
            if self.isStrict():
                raise Exception("Tried to update with inconsistent value")
            else:
                return False
            


    def __iter__(self):
        cur = 0

        while cur < len(self):
            yield self[cur]
            cur += 1

    def __contains__(self , time):
        index = self._lookup_time(CTime(time))
        if index >= 0:
            return True
        else:
            return False


    def lookupTime(self , time , tolerance_seconds_before = 0, tolerance_seconds_after = 0):
        """Will look up the report step corresponding to input @time.
        
        If the tolerance arguments tolerance_seconds_before and
        tolerance_seconds_after have the default value zero we require
        an exact match between input time argument and the content of
        the time map. 

        If the tolerance arguments are supplied the function will
        search through the time_map for the report step closest to the
        time argument, which satisfies the tolerance criteria.  

        With the call:

            lookupTime( datetime.date(2010,1,10) , 3600*24 , 3600*7)

        We will find the report step in the date interval 2010,1,9 -
        2010,1,17 which is closest to 2010,1,10. The tolerance limits
        are inclusive.

        If no report step satisfying the criteria is found a
        ValueError exception will be raised.

        """
        if tolerance_seconds_before == 0 and tolerance_seconds_after == 0:
            index = self._lookup_time(CTime(time))
        else:
            index = self._lookup_time_with_tolerance(CTime(time) , tolerance_seconds_before , tolerance_seconds_after)

        if index >= 0:
            return index
        else:
            raise ValueError("The time:%s was not found in the time_map instance" % time)


    def lookupDays(self , days):
        index = self._lookup_days(days)
        if index >= 0:
            return index
        else:
            raise ValueError("The days: %s was not found in the time_map instance" % days)
            

    def __len__(self):
        """ @rtype: int """
        return self._size()

    def free(self):
        self._free()

    def __repr__(self):
        ls = len(self)
        la = self.getLastStep()
        st = 'strict' if self.isStrict() else 'not strict'
        cnt = 'size = %d, last_step = %d, %s' % (ls, la, st)
        return self._create_repr(cnt)

    def dump(self):
        """ 
        Will return a list of tuples (step , CTime , days).
        """
        step_list = []
        for step,t in enumerate(self):
            step_list.append( (step , t , self.getSimulationDays( step )) )
        return step_list


    def getLastStep(self):
        return self._last_step()


    def upgrade107(self, refcase):
        self._upgrade107(refcase)

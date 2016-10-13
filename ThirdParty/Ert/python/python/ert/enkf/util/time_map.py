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

from cwrap import CWrapper, BaseCClass
from ert.enkf import ENKF_LIB
from ert.util import CTime


class TimeMap(BaseCClass):
    def __init__(self, filename = None):
        c_ptr = TimeMap.cNamespace().alloc()
        super(TimeMap, self).__init__(c_ptr)
        if filename:
            self.load(filename)


    def load(self, filename):
        if os.path.isfile( filename ):
            TimeMap.cNamespace().load(self , filename)
        else:
            raise IOError(( errno.ENOENT , "File not found: %s" % filename))


    def fwrite(self, filename):
        TimeMap.cNamespace().save(self , filename)


    def fload(self , filename):
        """
        Will load a timemap as a formatted file consisting of a list of dates: DD/MM/YYYY
        """
        if os.path.isfile( filename ):
            OK = TimeMap.cNamespace().fload(self , filename)
            if not OK:
                raise Exception("Error occured when loading timemap from:%s" % filename)
        else:
            raise IOError(( errno.ENOENT , "File not found: %s" % filename))



    def isStrict(self):
        return TimeMap.cNamespace().is_strict( self )


    def setStrict(self , strict):
        return TimeMap.cNamespace().set_strict( self , strict)
        

    def getSimulationDays(self, step):
        """ @rtype: double """
        if not isinstance(step, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if step < 0 or step >= size:
            raise IndexError("Index out of range: 0 <= %d < %d" % (step, size))

        return TimeMap.cNamespace().iget_sim_days(self, step)


    def __getitem__(self, index):
        """ @rtype: CTime """
        if not isinstance(index, int):
            raise TypeError("Expected an integer")

        size = len(self)
        if index < 0 or index >= size:
            raise IndexError("Index out of range: 0 <= %d < %d" % (index, size))

        return TimeMap.cNamespace().iget(self, index)

    def __setitem__(self , index , time):
        self.update( index , time )


    def update(self , index , time):
        if TimeMap.cNamespace().try_update(self , index , CTime(time)):
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
        index = TimeMap.cNamespace().lookup_time(self , CTime(time))
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
            index = TimeMap.cNamespace().lookup_time(self , CTime(time))
        else:
            index = TimeMap.cNamespace().lookup_time_with_tolerance(self , CTime(time) , tolerance_seconds_before , tolerance_seconds_after)

        if index >= 0:
            return index
        else:
            raise ValueError("The time:%s was not found in the time_map instance" % time)


    def lookupDays(self , days):
        index = TimeMap.cNamespace().lookup_days(self , days)
        if index >= 0:
            return index
        else:
            raise ValueError("The days: %s was not found in the time_map instance" % days)
            

    def __len__(self):
        """ @rtype: int """
        return TimeMap.cNamespace().size(self)

    def free(self):
        TimeMap.cNamespace().free(self)


    def dump(self):
        """ 
        Will return a list of tuples (step , CTime , days).
        """
        step_list = []
        for step,t in enumerate(self):
            step_list.append( (step , t , self.getSimulationDays( step )) )
        return step_list


    def getLastStep(self):
        return TimeMap.cNamespace().last_step(self)


    def upgrade107(self, refcase):
        TimeMap.cNamespace().upgrade107(self, refcase)

    
##################################################################
cwrapper = CWrapper(ENKF_LIB)
cwrapper.registerType("time_map", TimeMap)
cwrapper.registerType("time_map_obj", TimeMap.createPythonObject)
cwrapper.registerType("time_map_ref", TimeMap.createCReference)


##################################################################
##################################################################

TimeMap.cNamespace().free = cwrapper.prototype("void time_map_free( time_map )")
TimeMap.cNamespace().fread_alloc_readonly = cwrapper.prototype("c_void_p time_map_fread_alloc_readonly(char*)")
TimeMap.cNamespace().alloc = cwrapper.prototype("c_void_p time_map_alloc()")
TimeMap.cNamespace().load = cwrapper.prototype("bool time_map_fread(time_map , char*)")
TimeMap.cNamespace().save = cwrapper.prototype("void time_map_fwrite(time_map , char*)")
TimeMap.cNamespace().fload = cwrapper.prototype("bool time_map_fscanf(time_map , char*)")
TimeMap.cNamespace().iget_sim_days = cwrapper.prototype("double time_map_iget_sim_days(time_map, int)")
TimeMap.cNamespace().iget = cwrapper.prototype("time_t time_map_iget(time_map, int)")
TimeMap.cNamespace().size = cwrapper.prototype("int time_map_get_size(time_map)")
TimeMap.cNamespace().try_update = cwrapper.prototype("bool time_map_try_update(time_map , int , time_t)")
TimeMap.cNamespace().is_strict = cwrapper.prototype("bool time_map_is_strict( time_map )")
TimeMap.cNamespace().set_strict = cwrapper.prototype("void time_map_set_strict( time_map , bool)")
TimeMap.cNamespace().lookup_time = cwrapper.prototype("int time_map_lookup_time( time_map , time_t)")
TimeMap.cNamespace().lookup_time_with_tolerance = cwrapper.prototype("int time_map_lookup_time_with_tolerance( time_map , time_t , int , int)")
TimeMap.cNamespace().lookup_days = cwrapper.prototype("int time_map_lookup_days( time_map , double)")
TimeMap.cNamespace().last_step = cwrapper.prototype("int time_map_get_last_step( time_map )")
TimeMap.cNamespace().upgrade107 = cwrapper.prototype("void time_map_summary_upgrade107( time_map , ecl_sum )")

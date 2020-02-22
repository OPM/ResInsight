#  Copyright (C) 2014  Equinor ASA, Norway.
#
#  The file 'vector_template.py' is part of ERT - Ensemble based Reservoir Tool.
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
import datetime
import re

from ecl import EclPrototype
from ecl.util.util import VectorTemplate, CTime


class TimeVector(VectorTemplate):
    TYPE_NAME = "time_t_vector"
    default_format = "%d"

    _alloc               = EclPrototype("void*   time_t_vector_alloc(int, time_t )" , bind = False)
    _alloc_copy          = EclPrototype("time_t_vector_obj time_t_vector_alloc_copy(time_t_vector )")
    _strided_copy        = EclPrototype("time_t_vector_obj time_t_vector_alloc_strided_copy(time_t_vector , int , int , int)")
    _free                = EclPrototype("void   time_t_vector_free( time_t_vector )")
    _iget                = EclPrototype("time_t   time_t_vector_iget( time_t_vector , int )")
    _safe_iget           = EclPrototype("time_t   time_t_vector_safe_iget( time_t_vector , int )")
    _iset                = EclPrototype("time_t   time_t_vector_iset( time_t_vector , int , time_t)")
    _size                = EclPrototype("int      time_t_vector_size( time_t_vector )")
    _append              = EclPrototype("void     time_t_vector_append( time_t_vector , time_t )")
    _idel_block          = EclPrototype("void     time_t_vector_idel_block( time_t_vector , int , int )")
    _idel                = EclPrototype("void     time_t_vector_idel( time_t_vector , int )")
    _pop                 = EclPrototype("time_t   time_t_vector_pop( time_t_vector )")
    _lshift              = EclPrototype("void     time_t_vector_lshift( time_t_vector , int )")
    _rshift              = EclPrototype("void     time_t_vector_rshift( time_t_vector , int )")
    _insert              = EclPrototype("void     time_t_vector_insert( time_t_vector , int , time_t)")
    _fprintf             = EclPrototype("void     time_t_vector_fprintf( time_t_vector , FILE , char* , char*)")
    _sort                = EclPrototype("void     time_t_vector_sort( time_t_vector )")
    _rsort               = EclPrototype("void     time_t_vector_rsort( time_t_vector )")
    _reset               = EclPrototype("void     time_t_vector_reset( time_t_vector )")
    _set_read_only       = EclPrototype("void     time_t_vector_set_read_only( time_t_vector , bool )")
    _get_read_only       = EclPrototype("bool     time_t_vector_get_read_only( time_t_vector )")
    _get_max             = EclPrototype("time_t   time_t_vector_get_max( time_t_vector )")
    _get_min             = EclPrototype("time_t   time_t_vector_get_min( time_t_vector )")
    _get_max_index       = EclPrototype("int      time_t_vector_get_max_index( time_t_vector , bool)")
    _get_min_index       = EclPrototype("int      time_t_vector_get_min_index( time_t_vector , bool)")
    _shift               = EclPrototype("void     time_t_vector_shift( time_t_vector , time_t )")
    _scale               = EclPrototype("void     time_t_vector_scale( time_t_vector , time_t )")
    _div                 = EclPrototype("void     time_t_vector_div( time_t_vector , time_t )")
    _inplace_add         = EclPrototype("void     time_t_vector_inplace_add( time_t_vector , time_t_vector )")
    _inplace_mul         = EclPrototype("void     time_t_vector_inplace_mul( time_t_vector , time_t_vector )")
    _assign              = EclPrototype("void     time_t_vector_set_all( time_t_vector , time_t)")
    _memcpy              = EclPrototype("void     time_t_vector_memcpy(time_t_vector , time_t_vector )")
    _set_default         = EclPrototype("void     time_t_vector_set_default( time_t_vector , time_t)")
    _get_default         = EclPrototype("time_t   time_t_vector_get_default( time_t_vector )")
    _element_size        = EclPrototype("int      time_t_vector_element_size( time_t_vector )")

    _permute             = EclPrototype("void time_t_vector_permute(time_t_vector, permutation_vector)")
    _sort_perm           = EclPrototype("permutation_vector_obj time_t_vector_alloc_sort_perm(time_t_vector)")
    _rsort_perm          = EclPrototype("permutation_vector_obj time_t_vector_alloc_rsort_perm(time_t_vector)")
    _contains            = EclPrototype("bool time_t_vector_contains(time_t_vector, time_t)")
    _select_unique       = EclPrototype("void time_t_vector_select_unique(time_t_vector)")
    _element_sum         = EclPrototype("time_t time_t_vector_sum(time_t_vector)")
    _count_equal         = EclPrototype("int time_t_vector_count_equal(time_t_vector, time_t)")
    _init_range          = EclPrototype("void time_t_vector_init_range(time_t_vector, time_t , time_t , time_t)")
    _init_linear         = EclPrototype("bool time_t_vector_init_linear(time_t_vector, time_t, time_t, int)")
    _equal               = EclPrototype("bool time_t_vector_equal(time_t_vector, time_t_vector)")
    _first_eq            = EclPrototype("int time_t_vector_first_equal(time_t_vector, time_t_vector, int)")
    _first_neq           = EclPrototype("int time_t_vector_first_not_equal(time_t_vector, time_t_vector, int)")



    def __init__(self, default_value=None, initial_size=0):
        if default_value is None:
            super(TimeVector, self).__init__(CTime(0), initial_size)
        else:
            try:
                default = CTime(default_value)
            except:
                raise ValueError("default value invalid - must be type ctime() or date/datetime")

            super(TimeVector, self).__init__(default, initial_size)

    @classmethod
    def parseTimeUnit(cls, deltaString):
        deltaRegexp = re.compile("(?P<num>\d*)(?P<unit>[dmy])", re.IGNORECASE)
        matchObj = deltaRegexp.match(deltaString)
        if matchObj:
            try:
                num = int(matchObj.group("num"))
            except:
                num = 1

            timeUnit = matchObj.group("unit").lower()
            return num, timeUnit
        else:
            raise TypeError("The delta string must be on form \'1d\', \'2m\', \'Y\' for one day, two months or one year respectively")

    def __str__(self):
        """
        Returns string representantion of vector.
        """
        string_list = []
        for d in self:
            string_list.append("%s" % d)

        return str(string_list)

    def append(self, value):
        self._append(CTime(value))

    def __contains__(self, value):
        return self._contains(CTime(value))

    def nextTime(self, num, timeUnit):
        currentTime = self[-1].datetime()
        hour = currentTime.hour
        minute = currentTime.minute
        second = currentTime.second

        if timeUnit == "d":
            td = datetime.timedelta(days=num)
            currentTime += td
        else:
            day = currentTime.day
            month = currentTime.month
            year = currentTime.year

            if timeUnit == "y":
                year += num
            else:
                month += num - 1
                (deltaYear, newMonth) = divmod(month, 12)
                month = newMonth + 1
                year += deltaYear
            currentTime = datetime.datetime(year, month, day, hour, minute, second)

        return currentTime

    def appendTime(self, num, timeUnit):
        next = self.nextTime(num, timeUnit)
        self.append(CTime(next))

    @classmethod
    def createRegular(cls, start, end, deltaString):
        """
        The last element in the vector will be <= end; i.e. if the
        question of whether the range is closed in the upper end
        depends on the commensurability of the [start,end] interval
        and the delta:

        createRegular(0 , 10 , delta=3) => [0,3,6,9]
        createRegular(0 , 10 , delta=2) => [0,2,4,6,8,10]
        """
        start = CTime(start)
        end = CTime(end)
        if start > end:
            raise ValueError("The time interval is invalid start is after end")

        (num, timeUnit) = cls.parseTimeUnit(deltaString)

        timeVector = TimeVector()
        currentTime = start
        while currentTime <= end:
            ct = CTime(currentTime)
            timeVector.append(ct)
            currentTime = timeVector.nextTime(num, timeUnit)

        return timeVector

    def getDataPtr(self):
        raise NotImplementedError("The getDataPtr() function is not implemented for time_t vectors")

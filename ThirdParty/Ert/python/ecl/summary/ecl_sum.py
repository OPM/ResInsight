#  Copyright (C) 2011  Statoil ASA, Norway.
#
#  The file 'ecl_sum.py' is part of ERT - Ensemble based Reservoir Tool.
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
Module for loading and querying summary data.

The low-level organisation of summary data is extensively documented
in the C source files ecl_sum.c, ecl_smspec.c and ecl_sum_data in the
libecl/src directory.
"""

import warnings
import numpy
import datetime
import os.path
import ctypes

# Observe that there is some convention conflict with the C code
# regarding order of arguments: The C code generally takes the time
# index as the first argument and the key/key_index as second
# argument. In the python code this order has been reversed.
from cwrap import BaseCClass, CFILE

from ecl.util.util import monkey_the_camel
from ecl.util.util import StringList, CTime, DoubleVector, TimeVector, IntVector

from ecl.summary import EclSumTStep
from ecl.summary import EclSumVarType
from ecl.summary.ecl_sum_vector import EclSumVector
from ecl.summary.ecl_smspec_node import EclSMSPECNode
from ecl import EclPrototype, EclUnitTypeEnum
#, EclSumKeyWordVector




#import ecl.ecl_plot.sum_plot as sum_plot

# The date2num function is a verbatim copy of the _to_ordinalf()
# function from the matplotlib.dates module. Inserted here only to
# avoid importing the full matplotlib library. The date2num
# implementation could be replaced with:
#
#   from matplotlib.dates import date2num


HOURS_PER_DAY     = 24.0
MINUTES_PER_DAY   = 60 * HOURS_PER_DAY
SECONDS_PER_DAY   = 60 * MINUTES_PER_DAY
MUSECONDS_PER_DAY = 1e6 * SECONDS_PER_DAY

def date2num(dt):
    """
    Convert a python datetime instance to UTC float days.

    Convert datetime to the Gregorian date as UTC float days,
    preserving hours, minutes, seconds and microseconds, return value
    is a float. The function is a verbatim copy of the _to_ordinalf()
    function from the matplotlib.dates module.
    """

    if hasattr(dt, 'tzinfo') and dt.tzinfo is not None:
        delta = dt.tzinfo.utcoffset(dt)
        if delta is not None:
            dt -= delta

    base =  float(dt.toordinal())
    if hasattr(dt, 'hour'):
        base += (dt.hour/HOURS_PER_DAY     +
                 dt.minute/MINUTES_PER_DAY +
                 dt.second/SECONDS_PER_DAY +
                 dt.microsecond/MUSECONDS_PER_DAY)
    return base

class EclSum(BaseCClass):
    TYPE_NAME = "ecl_sum"
    _fread_alloc_case2             = EclPrototype("void*     ecl_sum_fread_alloc_case2__(char*, char*, bool, bool, int)", bind=False)
    _fread_alloc                   = EclPrototype("void*     ecl_sum_fread_alloc(char*, stringlist, char*, bool)", bind=False)
    _create_restart_writer         = EclPrototype("ecl_sum_obj  ecl_sum_alloc_restart_writer2(char*, char*, int, bool, bool, char*, time_t, bool, int, int, int)", bind = False)
    _create_writer                 = EclPrototype("ecl_sum_obj  ecl_sum_alloc_writer(char*, bool, bool, char*, time_t, bool, int, int, int)", bind = False)
    _resample                      = EclPrototype("ecl_sum_obj  ecl_sum_alloc_resample( ecl_sum, char*, time_t_vector)")
    _iiget                         = EclPrototype("double   ecl_sum_iget(ecl_sum, int, int)")
    _free                          = EclPrototype("void     ecl_sum_free(ecl_sum)")
    _data_length                   = EclPrototype("int      ecl_sum_get_data_length(ecl_sum)")
    _iget_sim_days                 = EclPrototype("double   ecl_sum_iget_sim_days(ecl_sum, int) ")
    _iget_report_step              = EclPrototype("int      ecl_sum_iget_report_step(ecl_sum, int) ")
    _iget_sim_time                 = EclPrototype("time_t   ecl_sum_iget_sim_time(ecl_sum, int) ")
    _get_report_end                = EclPrototype("int      ecl_sum_iget_report_end(ecl_sum, int)")
    _get_general_var               = EclPrototype("double   ecl_sum_get_general_var(ecl_sum, int, char*)")
    _get_general_var_index         = EclPrototype("int      ecl_sum_get_general_var_params_index(ecl_sum, char*)")
    _get_general_var_from_sim_days = EclPrototype("double   ecl_sum_get_general_var_from_sim_days(ecl_sum, double, char*)")
    _get_general_var_from_sim_time = EclPrototype("double   ecl_sum_get_general_var_from_sim_time(ecl_sum, time_t, char*)")
    _solve_days                    = EclPrototype("double_vector_obj  ecl_sum_alloc_days_solution(ecl_sum, char*, double, bool)")
    _solve_dates                   = EclPrototype("time_t_vector_obj  ecl_sum_alloc_time_solution(ecl_sum, char*, double, bool)")
    _get_first_gt                  = EclPrototype("int      ecl_sum_get_first_gt(ecl_sum, int, double)")
    _get_first_lt                  = EclPrototype("int      ecl_sum_get_first_lt(ecl_sum, int, double)")
    _get_start_date                = EclPrototype("time_t   ecl_sum_get_start_time(ecl_sum)")
    _get_end_date                  = EclPrototype("time_t   ecl_sum_get_end_time(ecl_sum)")
    _get_last_report_step          = EclPrototype("int      ecl_sum_get_last_report_step(ecl_sum)")
    _get_first_report_step         = EclPrototype("int      ecl_sum_get_first_report_step(ecl_sum)")
    _select_matching_keys          = EclPrototype("void     ecl_sum_select_matching_general_var_list(ecl_sum, char*, stringlist)")
    _has_key                       = EclPrototype("bool     ecl_sum_has_general_var(ecl_sum, char*)")
    _check_sim_time                = EclPrototype("bool     ecl_sum_check_sim_time(ecl_sum, time_t)")
    _check_sim_days                = EclPrototype("bool     ecl_sum_check_sim_days(ecl_sum, double)")
    _sim_length                    = EclPrototype("double   ecl_sum_get_sim_length(ecl_sum)")
    _get_first_day                 = EclPrototype("double   ecl_sum_get_first_day(ecl_sum)")
    _get_data_start                = EclPrototype("time_t   ecl_sum_get_data_start(ecl_sum)")
    _get_unit                      = EclPrototype("char*    ecl_sum_get_unit(ecl_sum, char*)")
    _get_restart_case              = EclPrototype("ecl_sum_ref ecl_sum_get_restart_case(ecl_sum)")
    _get_restart_step              = EclPrototype("int      ecl_sum_get_restart_step(ecl_sum)")
    _get_simcase                   = EclPrototype("char*    ecl_sum_get_case(ecl_sum)")
    _get_unit_system               = EclPrototype("ecl_unit_enum ecl_sum_get_unit_system(ecl_sum)")
    _get_base                      = EclPrototype("char*    ecl_sum_get_base(ecl_sum)")
    _get_path                      = EclPrototype("char*    ecl_sum_get_path(ecl_sum)")
    _get_abs_path                  = EclPrototype("char*    ecl_sum_get_abs_path(ecl_sum)")
    _get_report_step_from_time     = EclPrototype("int      ecl_sum_get_report_step_from_time(ecl_sum, time_t)")
    _get_report_step_from_days     = EclPrototype("int      ecl_sum_get_report_step_from_days(ecl_sum, double)")
    _get_report_time               = EclPrototype("time_t   ecl_sum_get_report_time(ecl_sum, int)")
    _fwrite_sum                    = EclPrototype("void     ecl_sum_fwrite(ecl_sum)")
    _can_write                     = EclPrototype("bool     ecl_sum_can_write(ecl_sum)")
    _set_case                      = EclPrototype("void     ecl_sum_set_case(ecl_sum, char*)")
    _alloc_time_vector             = EclPrototype("time_t_vector_obj ecl_sum_alloc_time_vector(ecl_sum, bool)")
    _alloc_data_vector             = EclPrototype("double_vector_obj ecl_sum_alloc_data_vector(ecl_sum, int, bool)")
    _get_var_node                  = EclPrototype("smspec_node_ref ecl_sum_get_general_var_node(ecl_sum, char*)")
    _create_well_list              = EclPrototype("stringlist_obj ecl_sum_alloc_well_list(ecl_sum, char*)")
    _create_group_list             = EclPrototype("stringlist_obj ecl_sum_alloc_group_list(ecl_sum, char*)")
    _add_variable                  = EclPrototype("smspec_node_ref   ecl_sum_add_var(ecl_sum, char*, char*, int, char*, double)")
    _add_tstep                     = EclPrototype("ecl_sum_tstep_ref ecl_sum_add_tstep(ecl_sum, int, double)")
    _export_csv                    = EclPrototype("void ecl_sum_export_csv(ecl_sum, char*, stringlist, char*, char*)")
    _identify_var_type             = EclPrototype("ecl_sum_var_type ecl_sum_identify_var_type(char*)", bind = False)
    _is_rate                       = EclPrototype("bool smspec_node_identify_rate(char*)", bind = False)
    _is_total                      = EclPrototype("bool smspec_node_identify_total(char*, ecl_sum_var_type)", bind = False)
    _get_last_value                = EclPrototype("double ecl_sum_get_last_value_gen_key(ecl_sum, char*)")
    _get_first_value               = EclPrototype("double ecl_sum_get_first_value_gen_key(ecl_sum, char*)")
    _init_numpy_vector             = EclPrototype("void ecl_sum_init_double_vector(ecl_sum, char*, double*)")
    _init_numpy_vector_interp      = EclPrototype("void ecl_sum_init_double_vector_interp(ecl_sum, char*, time_t_vector, double*)")
    _init_numpy_datetime64         = EclPrototype("void ecl_sum_init_datetime64_vector(ecl_sum, int64*, int)")


    def __init__(self, load_case, join_string=":", include_restart=True, lazy_load=True, file_options=0):
        """Loads a new EclSum instance with summary data.

        Loads a new summary results from the ECLIPSE case given by
        argument @load_case; @load_case should be the basename of the ECLIPSE
        simulation you want to load. @load_case can contain a leading path
        component, and also an extension - the latter will be ignored.

        The @join_string is the string used when combining elements
        from the WGNAMES, KEYWORDS and NUMS vectors into a composit
        key; with @join_string == ":" the water cut in well OP_1 will
        be available as "WWCT:OP_1".

        If the @include_restart parameter is set to true the summary
        loader will, in the case of a restarted ECLIPSE simulation,
        try to load summary results also from the restarted case.

        If the @lazy_load parameter is set to true the loader will not load all
        the data from a UNSMRY file at creation time, but wait until the data
        is actually requested. This will reduce startup time and memory usage,
        whereas getting a vector will be slower. When the summary data is split
        over multiple CASE.Snnn files all the data will be loaded at
        construction time, and the @lazy_load option is ignored. If the
        lazy_load functionality is used the file_options intege flag is passed
        when opening the UNSMRY file.

        """
        if not load_case:
            raise ValueError('load_case must be the basename of the simulation')
        c_pointer = self._fread_alloc_case2(load_case, join_string, include_restart, lazy_load, file_options)
        if c_pointer is None:
            raise IOError("Failed to create summary instance from argument:%s" % load_case)

        super(EclSum, self).__init__(c_pointer)
        self._load_case = load_case


    @classmethod
    def load(cls, smspec_file, unsmry_file, key_join_string = ":", include_restart = True):
        if not os.path.isfile( smspec_file ):
            raise IOError("No such file: %s" % smspec_file)

        if not os.path.isfile( unsmry_file ):
            raise IOError("No such file: %s" % unsmry_file )

        data_files = StringList( )
        data_files.append( unsmry_file )
        c_ptr = cls._fread_alloc(smspec_file, data_files, key_join_string, include_restart)
        if c_ptr is None:
            raise IOError("Failed to create summary instance")

        ecl_sum = cls.createPythonObject( c_ptr )
        ecl_sum._load_case = smspec_file
        return ecl_sum


    @classmethod
    def createCReference(cls, c_pointer, parent=None):
        result = super(EclSum, cls).createCReference(c_pointer, parent)
        return result


    @classmethod
    def createPythonObject(cls, c_pointer):
        result = super(EclSum, cls).createPythonObject(c_pointer)
        return result


    @staticmethod
    def var_type(keyword):
        return EclSum._identify_var_type(keyword)

    @staticmethod
    def is_rate(keyword):
        return EclSum._is_rate(keyword)

    @staticmethod
    def is_total(keyword):
        return EclSum._is_total(keyword, EclSum.var_type(keyword))

    @staticmethod
    def writer(case,
               start_time,
               nx,ny,nz,
               fmt_output=False,
               unified=True,
               time_in_days=True,
               key_join_string=":"):
        """
        The writer is not generally usable.
        @rtype: EclSum
        """

        start = CTime(start_time)

        smry = EclSum._create_writer(case,
                                     fmt_output,
                                     unified,
                                     key_join_string,
                                     start,
                                     time_in_days,
                                     nx,
                                     ny,
                                     nz)
        smry._load_case = 'writer'
        return smry


    @staticmethod
    def restart_writer(case,
                       restart_case,
                       restart_step,
                       start_time,
                       nx,ny,nz,
                       fmt_output=False,
                       unified=True,
                       time_in_days=True,
                       key_join_string=":"):
        """
        The writer is not generally usable.
        @rtype: EclSum
        """

        start = CTime(start_time)

        smry = EclSum._create_restart_writer(case,
                                             restart_case,
                                             restart_step,
                                             fmt_output,
                                             unified,
                                             key_join_string,
                                             start,
                                             time_in_days,
                                             nx,
                                             ny,
                                             nz)
        smry._load_case = 'restart_writer'
        return smry

    def add_variable(self, variable, wgname=None, num=0, unit="None", default_value=0):
        return self._add_variable(variable, wgname, num, unit, default_value).setParent(parent=self)


    def add_t_step(self, report_step, sim_days):
        """ @rtype: EclSumTStep """
        # report_step int
        if not isinstance(report_step, int):
            raise TypeError('Parameter report_step should be int, was %r' % report_step)
        try:
            float(sim_days)
        except TypeError:
            raise TypeError('Parameter sim_days should be float, was %r' % sim_days)

        sim_seconds = sim_days * 24 * 60 * 60

        return self._add_tstep(report_step, sim_seconds).setParent(parent=self)




    def get_vector(self, key, report_only=False):
        """
        Will return EclSumVector according to @key.

        Will raise exception KeyError if the summary object does not
        have @key.
        """
        warnings.warn("The method get_vector() has been deprecated, use numpy_vector() instead", DeprecationWarning)
        self.assertKeyValid(key)
        if report_only:
            return EclSumVector(self, key, report_only=True)
        else:
            return EclSumVector(self, key)


    def report_index_list(self):
        """
        Internal function for working with report_steps.
        """
        first_report = self.first_report
        last_report  = self.last_report
        index_list = IntVector()
        for report_step in range(first_report, last_report + 1):
            time_index = self._get_report_end(report_step)
            index_list.append(time_index)
        return index_list



    def wells(self, pattern=None):
        """
        Will return a list of all the well names in case.

        If the pattern variable is different from None only wells
        matching the pattern will be returned; the matching is based
        on fnmatch(), i.e. shell style wildcards.
        """
        return self._create_well_list(pattern)


    def groups(self, pattern=None):
        """
        Will return a list of all the group names in case.

        If the pattern variable is different from None only groups
        matching the pattern will be returned; the matching is based
        on fnmatch(), i.e. shell style wildcards.
        """
        return self._create_group_list(pattern)


    def get_values(self, key, report_only=False):
        """
        Will return numpy vector of all values according to @key.

        If the optional argument report_only is true only the values
        corresponding to report steps are included.  The method is
        also available as the 'values' property of an EclSumVector
        instance.
        """
        warnings.warn("The method get_values() has been deprecated - use numpy_vector() instead.", DeprecationWarning)
        if self.has_key(key):
            key_index = self._get_general_var_index(key)
            if report_only:
                index_list = self.report_index_list()
                values = numpy.zeros(len(index_list))
                for i in range(len(index_list)):
                    time_index = index_list[i]
                    values[i]  = self._iiget(time_index, key_index)
            else:
                length = self._data_length()
                values = numpy.zeros(length)
                for i in range(length):
                    values[i] = self._iiget(i, key_index)

            return values
        else:
            raise KeyError("Summary object does not have key:%s" % key)

    def _make_time_vector(self, time_index):
        time_points = TimeVector()
        for t in time_index:
            time_points.append(t)
        return time_points

    def numpy_vector(self, key, time_index=None, report_only=False):
        """Will return numpy vector of all the values corresponding to @key.

        The optional argument @time_index can be used to limit the time points
        where you want evaluation. The time_index argument should be a list of
        datetime instances. The values will be interpolated to the time points
        given in the time_index vector. If the time points in the time_inedx
        vector are outside of the simulated range you will get an extrapolated
        value:

             Rates    -> 0
             Not rate -> first or last simulated value.

        The function will raise KeyError if the requested key does not exist.
        If many keys are needed it will be faster to use the pandas_frame()
        function.

        If you set the optional argument report_only to True the you will only
        get values at the report dates. Observe that passing report_only=True
        can not be combined with a value for time_index, that will give you a
        ValueError exception.

        """
        if key not in self:
            raise KeyError("No such key:%s" % key)

        if report_only:
            if time_index is None:
                time_index = self.report_dates
            else:
                raise ValueError("Can not suuply both time_index and report_only=True")

        if time_index is None:
            np_vector = numpy.zeros(len(self))
            self._init_numpy_vector(key ,np_vector.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
            return np_vector
        else:
           time_vector = self._make_time_vector(time_index)
           np_vector = numpy.zeros(len(time_vector))
           self._init_numpy_vector_interp(key, time_vector, np_vector.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
           return np_vector


    @property
    def numpy_dates(self):
        """
        Will return numpy vector of numpy.datetime64() values for all the simulated timepoints.
        """
        np_dates = numpy.zeros(len(self), dtype="datetime64[ms]")
        self._init_numpy_datetime64(np_dates.ctypes.data_as(ctypes.POINTER(ctypes.c_int64)), 1000)
        return np_dates


    @property
    def dates(self):
        """
        Will return ordinary Python list of datetime.datetime() objects of simulated timepoints.
        """
        np_dates = self.numpy_dates
        return np_dates.tolist()


    @property
    def report_dates(self):
        dates = []
        if len(self):
            for report in range(self.first_report,self.last_report + 1):
                dates.append(self.get_report_time( report ))
        return dates


    def pandas_frame(self, time_index = None, column_keys = None):
        """Will create a pandas frame with summary data.

        By default you will get all time points in the summary case, but by
        using the time_index argument you can control which times you are
        interested in. If you have supplied a time_index argument the data will
        be interpolated to these time values. If the time points in the
        time_index vector are outside of the simulated range you will get an
        extrapolated value:

             Rates    -> 0
             Not rate -> first or last simulated value.


        By default the frame will contain all the summary vectors in the case,
        but this can be controlled by using the column_keys argument. The
        column_keys should be a list of strings, and each summary vector
        matching one of the elements in the @column_keys will get a column in
        the frame, you can use wildcards like "WWCT:*" and "*:OP". If you
        supply a column_keys argument which does not resolve to any valid
        summary keys you will get a ValueError exception.


          sum = EclSum(case)
          monthly_dates = sum.time_range(interval="1M")
          data = sum.pandas_frame(time_index = monthly_dates, column_keys=["F*PT"])

                            FOPT       FGPT      FWPT
              2010-01-01    100.7      200.0     25.0
              2010-02-01    150.7      275.0     67.6
              2010-03-01    276.7      310.6     67.0
              2010-04-01    672.7      620.4     78.7
              ....
        """
        from ecl.summary import EclSumKeyWordVector
        import pandas
        if column_keys is None:
            keywords = EclSumKeyWordVector(self, add_keywords = True)
        else:
            keywords = EclSumKeyWordVector(self)
            for key in column_keys:
                keywords.add_keywords(key)


        if len(keywords) == 0:
            raise ValueError("No valid key")

        if time_index is None:
            time_index = self.numpy_dates
            data = numpy.zeros([len(time_index), len(keywords)])
            EclSum._init_pandas_frame(self, keywords,data.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))
        else:
            time_points = self._make_time_vector(time_index)
            data = numpy.zeros([len(time_points), len(keywords)])
            EclSum._init_pandas_frame_interp(self, keywords, time_points, data.ctypes.data_as(ctypes.POINTER(ctypes.c_double)))

        frame = pandas.DataFrame(index = time_index, columns=list(keywords), data=data)
        return frame


    def get_key_index(self, key):
        """
        Lookup parameter index of @key.

        All the summary keys identified in the SMSPEC file have a
        corresponding index which is used internally. This function
        will return that index for input key @key, this can then be
        used in subsequent calls to e.g. the iiget() method. This is a
        minor optimization in the case of many lookups of the same
        key:

           sum = ecl.EclSum(case)
           key_index = sum.get_key_index(key)
           for time_index in range(sum.length):
               value = sum.iiget(time_index, key_index)

        Quite low-level function, should probably rather use a
        EclSumVector based function?
        """
        index = self._get_general_var_index(key)
        if index >= 0:
            return index
        else:
            return None


    def last_value(self, key):
        """
        Will return the last value corresponding to @key.

        Typically useful to get the total production at end of
        simulation:

           total_production = sum.last_value("FOPT")

        The alternative method 'last' will return a EclSumNode
        instance with some extra time related information.
        """
        if not key in self:
            raise KeyError("No such key:%s" % key)

        return self._get_last_value(key)


    def first_value(self, key):
        """
        Will return first value corresponding to @key.
        """
        if not key in self:
            raise KeyError("No such key:%s" % key)

        return self._get_first_value(key)

    def get_last_value(self,key):
        warnings.warn("The function get_last_value() is deprecated, use last_value() instead",DeprecationWarning)
        return self.last_value(key)

    def get_last(self, key):
        """
        Will return the last EclSumNode corresponding to @key.

        If you are only interested in the final value, you can use the
        last_value() method.
        """
        return self[key].last


    def iiget(self, time_index, key_index):
        """
        Lookup a summary value based on naive @time_index and
        @key_index.

        The iiget() method will lookup a summary value based on the
        'time' value give by @time_index (i.e. naive counting of
        time steps starting at zero), and a key index given by
        @key_index. The @key_index value will typically be obtained
        with the get_key_index() method first.

        This is a quite low level function, in most cases it will be
        natural to go via e.g. an EclSumVector instance.
        """
        return self._iiget(time_index, key_index)


    def iget(self, key, time_index):
        """
        Lookup summary value based on @time_index and key.

        The @time_index value should be an integer [0,num_steps) and
        @key should be string key. To get all the water cut values
        from a well:

            for time_index in range(sum.length):
                wwct = sum.iget(time_index, "WWCT:W5")

        This is a quite low level function, in most cases it will be
        natural to go via e.g. an EclSumVector instance.
        """
        return self._get_general_var(time_index, key)


    def __len__(self):
        """
        The number of timesteps in the dataset; the return when evaluating
        len(case).

        """
        return self._data_length()


    def __contains__(self, key):
        if self._has_key(key):
            return True
        else:
            return False

    def assert_key_valid(self, key):
        if not key in self:
            raise KeyError("The summary key:%s was not recognized" % key)

    def __iter__(self):
        return iter(self.keys())

    def __getitem__(self, key):
        """
        Implements [] operator - @key should be a summary key.

        The returned value will be a EclSumVector instance.
        """
        warnings.warn("The method the [] operator will change behaviour in the future. It will then return a plain numpy vector. You are advised to change to use the numpy_vector() method right away", DeprecationWarning)
        return self.get_vector(key)


    def scale_vector(self, key, scalar):
        msg = """The function EclSum.scale_vector has been removed. As an alternative you
are advised to fetch vector as a numpy vector and then scale that yourself:

    vec = ecl_sum.numpy_vector(key)
    vec *= scalar

        """
        raise NotImplementedError(msg)

    def shift_vector(self, key, addend):
        msg = """The function EclSum.shift_vector has been removed. As an alternative you
are advised to fetch vector as a numpy vector and then scale that yourself:

    vec = ecl_sum.numpy_vector(key)
    vec += scalar

        """
        raise NotImplementedError(msg)


    def check_sim_time(self, date):
        """
        Will check if the input date is in the time span [sim_start, sim_end].
        """
        if not isinstance(date, CTime):
            date = CTime(date)
        return self._check_sim_time(date)

    def get_interp_direct(self,key, date):

        if not isinstance(date, CTime):
            date = CTime(date)
        return self._get_general_var_from_sim_time(date, key)

    def get_interp(self, key, days=None, date=None):
        """
        Will lookup vector @key at time given by @days or @date.

        Requiers exactly one input argument @days or @date; will raise
        exception ValueError if this is not satisfied.

        The method will check that the time argument is within the
        time limits of the simulation; if else the method will raise
        exception ValueError.

        Also available as method get_interp() on the EclSumVector
        class.
        """
        self.assertKeyValid(key)
        if days is None and date is None:
            raise ValueError("Must supply either days or date")

        if days is None:
            t = CTime(date)
            if self.check_sim_time(t):
                return self._get_general_var_from_sim_time(t, key)
            else:

                raise ValueError("date:%s is outside range of simulation data" % date)
        elif date is None:
            if self._check_sim_days(days):
                return self._get_general_var_from_sim_days(days, key)
            else:
                raise ValueError("days:%s is outside range of simulation: [%g,%g]" % (days, self.first_day, self.sim_length))
        else:
            raise ValueError("Must supply either days or date")


    def get_interp_row(self, key_list, sim_time, invalid_value = -1):
        ctime = CTime(sim_time)
        data = DoubleVector( initial_size = len(key_list) , default_value = invalid_value)
        EclSum._get_interp_vector(self, ctime, key_list, data)
        return data


    def time_range(self, start=None, end=None, interval="1Y", num_timestep = None, extend_end=True):
        """Will create a vector of timepoints based on the current case.

        By default the timepoints will be regularly sampled based on the
        interval given by the @interval string. Alternatively the total number
        of timesteps can be specified, if the @num_timestep option is specified
        that will take presedence.
        """
        (num, timeUnit) = TimeVector.parseTimeUnit(interval)

        if start is None:
            start = self.getDataStartTime()
        else:
            if isinstance(start, datetime.date):
                start = datetime.datetime(start.year, start.month, start.day, 0, 0, 0)

            if start < self.getDataStartTime():
                start = self.getDataStartTime()


        if end is None:
            end = self.getEndTime()
        else:
            if isinstance(end, datetime.date):
                end = datetime.datetime(end.year, end.month, end.day, 0, 0, 0)

            if end > self.getEndTime():
                end = self.getEndTime()

        if end < start:
            raise ValueError("Invalid time interval start after end")


        if not num_timestep is None:
            return TimeVector.create_linear(CTime(start), CTime(end), num_timestep)

        range_start = start
        range_end = end
        if not timeUnit == "d":
            year1 = start.year
            year2 = end.year
            month1 = start.month
            month2 = end.month
            day1 = start.day
            day2 = end.day
            if extend_end:
                if timeUnit == 'm':
                    if day2 > 1:
                        month2 += 1
                        if month2 == 13:
                            year2 += 1
                            month2 = 1
                elif timeUnit == "y":
                    month1 = 1
                    if year2 > 1 or day2 > 1:
                        year2 += 1
                        month2 = 1
            day1 = 1
            day2 = 1

            range_start = datetime.date(year1, month1, day1)
            range_end =  datetime.date(year2, month2, day2)

        trange = TimeVector.createRegular(range_start, range_end, interval)

        # If the simulation does not start at the first of the month
        # the start value will be before the simulation start; we
        # manually shift the first element in the trange to the start
        # value; the same for the end of list.

        if trange[-1] < end:
            if extend_end:
                trange.appendTime(num, timeUnit)
            else:
                trange.append(end)

        data_start = self.getDataStartTime()
        if trange[0] < data_start:
            trange[0] = CTime(data_start)

        return trange



    def blocked_production(self, totalKey, timeRange):
        node = self.smspec_node(totalKey)
        if node.isTotal():
            total = DoubleVector()
            for t in timeRange:
                if t < CTime(self.start_time):
                    total.append(0)
                elif t >= CTime(self.end_time):
                    total.append(self.last_value(totalKey))
                else:
                    total.append(self.get_interp(totalKey, date=t))
            tmp = total << 1
            total.pop()
            return tmp - total
        else:
            raise TypeError("The blockedProduction method must be called with one of the TOTAL keys like e.g. FOPT or GWIT")


    def get_report(self, date=None, days=None):
        """
        Will return the report step corresponding to input @date or @days.

        If the input argument does not correspond to any report steps
        the function will return -1. Observe that the function
        requires strict equality.
        """
        if date:
            if days:
                raise ValueError("Must supply either days or date")
            step = self._get_report_step_from_time(CTime(date))
        elif days:
            step = self._get_report_step_from_days(days)

        return step


    def get_report_time(self, report):
        """
        Will return the datetime corresponding to the report_step @report.
        """
        return CTime(self._get_report_time(report)).date()


    def get_interp_vector(self, key, days_list=None, date_list=None):
        """
        Will return numpy vector with interpolated values.

        Requiers exactly one input argument @days or @date; will raise
        exception ValueError if this is not satisfied.

        The method will check that the time arguments are within the
        time limits of the simulation; if else the method will raise
        exception ValueError.

        Also available as method get_interp_vector() on the
        EclSumVector class.
        """
        self.assertKeyValid(key)
        if days_list:
            if date_list:
                raise ValueError("Must supply either days_list or date_list")
            else:
                vector = numpy.zeros(len(days_list))
                sim_length = self.sim_length
                sim_start  = self.first_day
                index = 0
                for days in days_list:
                    if (days >= sim_start) and (days <= sim_length):
                        vector[index] = self._get_general_var_from_sim_days(days, key)
                    else:
                        raise ValueError("Invalid days value")
                    index += 1
        elif date_list:
            start_time = self.data_start
            end_time   = self.end_date
            vector = numpy.zeros(len(date_list))
            index = 0

            for date in date_list:
                ct = CTime(date)
                if start_time <= ct <= end_time:
                    vector[index] =  self._get_general_var_from_sim_time(ct, key)
                else:
                    raise ValueError("Invalid date value")
                index += 1
        else:
            raise ValueError("Must supply either days_list or date_list")
        return vector


    def get_from_report(self, key, report_step):
        """
        Return summary value of @key at time @report_step.
        """
        time_index = self._get_report_end(report_step)
        return self._get_general_var(time_index, key)


    def has_key(self, key):
        """
        Check if summary object has key @key.
        """
        return key in self


    def smspec_node(self, key):
        """
        Will return a EclSMSPECNode instance corresponding to @key.

        The returned EclSMPECNode instance can then be used to ask for
        various properties of the variable; i.e. if it is a rate
        variable, what is the unit, if it is a total variable and so
        on.
        """
        if self.has_key(key):
            node = self._get_var_node(key).setParent(self)
            return node
        else:
            raise KeyError("Summary case does not have key:%s" % key)


    def unit(self, key):
        """
        Will return the unit of @key.
        """
        node = self.smspec_node(key)
        return node.unit


    @property
    def unit_system(self):
        """
        Will return the unit system in use for this case.
        """
        return self._get_unit_system()

    @property
    def case(self):
        """
        Will return the case name of the current instance - optionally including path.
        """
        return self._get_simcase()


    @property
    def restart_step(self):
        """
        Will return the report step this case has been restarted from, or -1.
        """
        return self._get_restart_step()


    @property
    def restart_case(self):
        restart_case = self._get_restart_case()
        if restart_case:
            restart_case.setParent(parent=self)
        return restart_case


    @property
    def path(self):
        """
        Will return the path to the current case. Will be None for
        case in CWD. See also abs_path.
        """
        return self._get_path()

    @property
    def base(self):
        """
        Will return the basename of the current case - no path.
        """
        return self._get_base()

    @property
    def abs_path(self):
        """
        Will return the absolute path to the current case.
        """
        return self._get_abs_path()

    #-----------------------------------------------------------------
    # Here comes functions for getting vectors of the time
    # dimension. All the get_xxx() functions have an optional boolean
    # argument @report_only. If this argument is set to True the
    # functions will return time vectors only corresponding to the
    # report times.
    #
    # In addition to the get_xxx() methods there are properties with
    # the same name (excluding the 'get'); these properties correspond
    # to an get_xxx() invocation with optional argument report_only
    # set to False (i.e. the defualt).

    @property
    def days(self):
        """
        Will return a numpy vector of simulations days.
        """
        return self.get_days(False)

    def get_days(self, report_only=False):
        """
        Will return a numpy vector of simulations days.

        If the optional argument @report_only is set to True, only
        'days' values corresponding to report steps will be included.
        """
        if report_only:
            dates = self.report_dates
            start_date = self.data_start
            start = datetime.date(start_date.year, start_date.month, start_date.day)
            return [ (x - start).total_seconds( ) / 86400 for x in dates ]
        else:
            return [ self._iget_sim_days(index) for index in range(len(self)) ]

    def get_dates(self, report_only=False):
        """
        Will return a list of simulation dates.

        The list will be an ordinary Python list, and the dates will
        be in terms ordinary Python datetime values. If the optional
        argument @report_only is set to True, only dates corresponding
        to report steps will be included.
        """
        if report_only:
            return self.report_dates
        else:
            return self.dates

    @property
    def mpl_dates(self):
        """
        Will return a numpy vector of dates ready for matplotlib

        The content of the vector are dates in matplotlib format,
        i.e. floats - generated by the date2num() function at the top
        of this file.
        """
        warnings.warn("The mpl_dates property has been deprecated - use numpy_dates instead", DeprecationWarning)
        return self.get_mpl_dates(False)


    def get_mpl_dates(self, report_only=False):
        """
        Will return a numpy vector of dates ready for matplotlib

        If the optional argument @report_only is set to True, only
        dates values corresponding to report steps will be
        included. The content of the vector are dates in matplotlib
        format, i.e. floats - generated by the date2num() function at
        the top of this file.
        """
        warnings.warn("The get_mpl_dates( ) method has been deprecated - use numpy_dates instead", DeprecationWarning)
        if report_only:
            return [ date2num(dt) for dt in self.report_dates ]
        else:
            return [ date2num(dt) for dt in self.dates ]


    @property
    def report_step(self):
        """
        Will return a list of report steps.

        The simulator will typically use several simulation timesteps
        for each report step, and the number will change between
        different report steps. So - assuming that the first report
        step one has five simulations timesteps and the next two have
        three the report_step vector can look like:

          [...,1,1,1,1,1,2,2,2,3,3,3,....]

        """
        return self.get_report_step(False)

    def get_report_step(self, report_only=False):
        if report_only:
            report_steps = list(range(self.first_report, self.last_report + 1))
        else:
            report_steps = []
            for index in range(len(self)):
                report_steps.append( self._iget_report_step(index) )

        return report_steps

    #-----------------------------------------------------------------

    def iget_days(self, time_index):
        """
        Returns the number of simulation days for element nr @time_index.
        """
        return self._iget_sim_days(time_index)

    def iget_date(self, time_index):
        """
        Returns the simulation date for element nr @time_index.
        """
        long_time = self._iget_sim_time(time_index)
        ct = CTime(long_time)
        return ct.datetime()


    def iget_report(self, time_index):
        """
        Returns the report step corresponding to @time_index.

        One report step will in general contain many ministeps.
        """
        return self._iget_report_step(time_index)


    @property
    def length(self):
        """
        The number of timesteps in the dataset.
        """
        return self._data_length()

    @property
    def first_day(self):
        """
        The first day we have simulation data for; normally 0.
        """
        return self._get_first_day()

    @property
    def sim_length(self):
        """Will return the total time span for the simulation data.

        The lengt will be returned in time unit used in the simulation data;
        i.e. typically days.
        """
        return self.getSimulationLength()

    @property
    def start_date(self):
        """
        A Python date instance with the start date.

        The start time is taken from the SMSPEC file, and in case not
        all timesteps have been loaded, e.g. for a restarted case, the
        returned start_date might be different from the datetime of
        the first (loaded) timestep.
        """
        ct = self._get_start_date()
        return CTime(ct).date()


    @property
    def end_date(self):
        """
        The date of the last (loaded) time step.
        """
        return CTime(self._get_end_date()).date()



    @property
    def data_start(self):
        return self.getDataStartTime()



    @property
    def end_time(self):
        """
        The time of the last (loaded) time step.
        """
        return self.getEndTime()


    @property
    def start_time(self):
        return self.getStartTime()


    def get_data_start_time(self):
        """The first date we have data for.

        Thiw will mostly be equal to getStartTime(), but in the case
        of restarts, where the case we have restarted from is not
        found, this time will be later than the true start of the
        field.
        """
        return CTime(self._get_data_start()).datetime()



    def get_start_time(self):
        """
        A Python datetime instance with the start time.

        See start_date() for further details.
        """
        return CTime(self._get_start_date()).datetime()


    def get_end_time(self):
        """
        A Python datetime instance with the last loaded time.
        """
        return CTime(self._get_end_date()).datetime()

    def getSimulationLength(self):
        """
        The length of the current dataset in simulation days.

        Will include the length of a leading restart section,
        irrespective of whether we have data for this or not.
        """
        return self._sim_length()



    @property
    def last_report(self):
        """
        The number of the last report step in the dataset.
        """
        return self._get_last_report_step()

    @property
    def first_report(self):
        """
        The number of the first report step in the dataset.
        """
        return self._get_first_report_step()

    def first_gt_index(self, key, limit):
        """
        Returns the first index where @key is above @limit.
        """
        key_index  = self._get_general_var_index(key)
        time_index = self._get_first_gt(key_index, limit)
        return time_index

    def first_lt_index(self, key, limit):
        """
        Returns the first index where @key is below @limit.
        """
        key_index  = self._get_general_var_index(key)
        time_index = self._get_first_lt(key_index, limit)
        return time_index

    def first_gt(self, key, limit):
        """
        First EclSumNode of @key which is above @limit.
        """
        vector = self[key]
        return vector.first_gt(limit)

    def first_lt(self, key, limit):
        """
        First EclSumNode of @key which is below @limit.
        """
        vector = self[key]
        return vector.first_lt(limit)

    def solve_dates(self, key, value, rates_clamp_lower=True):
        """Will solve the equation vector[@key] == value for dates.

        See solveDays() for further details.
        """
        if not key in self:
            raise KeyError("Unrecognized key:%s" % key)

        if len(self) < 2:
            raise ValueError("Must have at least two elements to start solving")

        return [ x.datetime() for x in self._solve_dates(key, value, rates_clamp_lower)]


    def solve_days(self, key, value, rates_clamp_lower=True):
        """Will solve the equation vector[@key] == value.

        This method will solve find tha approximate simulation days
        where the vector @key is equal @value. The method will return
        a list of values, which can have zero, one or multiple values:

          case = EclSum("CASE")
          days = case.solveDays("RPR:2", 200)

          if len(days) == 0:
             print("Pressure was never equal to 200 BARSA")
          elif len(days) == 1:
             print("Pressure equal to 200 BARSA after %s simulation days" % days[0])
          else:
             print("Pressure equal to 200 BARSA multiple times")
             for index,day in enumerate(days):
                 print("Solution[%d] : %s days" % (index, day))

        For variables like pressure and total volumes the solution is
        based on straightforward linear interpolation between the
        simulated values; that is quite intuitive. However - rates is
        less intuitive, and how a rate like FOPR is handled can be
        surprising:

        Fundamentally the simulator works with *volumes*. Assume that
        the simulator calculates that between the times t1 and t2 the
        total volume of oil produced is V, then the oil production
        rate is given as:

                   FOPR = V / (t2 - t1)

        This is the average production rate in the timespan (t1,t2];
        the simulator does not have any information on a finer time
        scale than this - so the natural assumption is that the
        production is constant at this value for the whole time
        period. The logical consequence of this is that production
        rates should be visualized as a piecewise constant function:



                                    A            B
                                    |            |
           /|\ OPR                  |            |
            |                      \|/          \|/
            |
            |                       +============X
            |                       |            |
            |-------------------------------------------------- X
            |                       |            |
            |         +=============X
            |                                    |
            |         |                          +===========X
            |=========X                                      |
            |
            +---------+-------------+------------+-----------+-->
            t0        t1            t2           t3          t4 time


        This figure shows a plot of the OPR as a piecewise constant
        function. In a strict mathematical sense the equation:

                               OPR = X

        Does not have a solution at all, but since this inequality:

                         OPR(t2) < X < OPR(t3)

        it is natural to say that the equation has a solution. The
        default behaviour is to say that the (first) solution in this
        case is:

                           tx = t2 + epsilon

        corresponding to the arrow 'A' on the figure. Alternatively if
        you set the optional argument 'rates_clamp_lower' to false the
        method will find the solution:

                      tx = t3

        corresponding to the arrow 'B* in the figure.

        """
        if not key in self:
            raise KeyError("Unrecognized key:%s" % key)

        if len(self) < 2:
            raise ValueError("Must have at least two elements to start solving")

        return self._solve_days(key, value, rates_clamp_lower)


    def keys(self, pattern=None):
        """
        Return a StringList of summary keys matching @pattern.

        The matching algorithm is ultimately based on the fnmatch()
        function, i.e. normal shell-character syntax is used. With
        @pattern == "WWCT:*" you will get a list of watercut keys for
        all wells.

        If pattern is None you will get all the keys of summary
        object.
        """
        s = StringList()
        self._select_matching_keys(pattern, s)
        return s


    def can_write(self):
        return self._can_write( )


    def fwrite(self, ecl_case=None):
        if not self.can_write():
            raise NotImplementedError("Write method is not implemented for this case. lazy_load=True??")

        if ecl_case:
            self._set_case(ecl_case)

        self._fwrite_sum()


    def alloc_time_vector(self, report_only):
        return self._alloc_time_vector(report_only)

    def alloc_data_vector(self, data_index, report_only):
        return self._alloc_data_vector(data_index, report_only)

    def get_general_var_index(self, key):
        return self._get_general_var_index(key)

    def free(self):
        self._free()

    def _nicename(self):
        """load_case is often full path to summary file,
        if so, output basename, else name
        """
        name = self._load_case
        if name and os.path.isfile(name):
            name = os.path.basename(name)
        return name

    def __repr__(self):
        """Returns, e.g.
           EclSum("NORNE_ATW2013.UNSMRY", [1997-11-06 00:00:00, 2006-12-01 00:00:00], keys=3781) at 0x1609e20
        """
        name = self._nicename()
        s_time   = self.getStartTime()
        e_time   = self.getEndTime()
        num_keys = len(self.keys())
        content = 'name="%s", time=[%s, %s], keys=%d' % (name, s_time, e_time, num_keys)
        return self._create_repr(content)

    def dump_csv_line(self, time, keywords, pfile):
        """
        Will dump a csv formatted line of the keywords in @keywords,
        evaluated at the intertpolated time @time. @pfile should point to an open Python file handle.
        """
        cfile = CFILE(pfile)
        ctime = CTime(time)
        EclSum._dump_csv_line(self, ctime, keywords, cfile)



    def export_csv(self, filename, keys=None, date_format="%Y-%m-%d", sep=";"):
        """Will create a CSV file with summary data.

        By default all the vectors in the summary case will be
        exported, but by using the optional keys parameter you can
        limit the keys which are exported:

          ecl_sum = EclSum("CASE")
          ecl_sum.exportCSV("case.csv", keys=["W*:OP1", "W*:OP2", "F*T"])

        Will export all well related variables for wells 'OP1' and
        'OP2' and all total field vectors.
        """

        if keys is None:
            var_list = self.keys()
        else:
            var_list = StringList()
            for key in keys:
                var_list |= self.keys(pattern=key)
        self._export_csv(filename, var_list, date_format, sep)



    def resample(self, new_case_name, time_points):
        return self._resample(new_case_name, time_points)


import ecl.summary.ecl_sum_keyword_vector
EclSum._dump_csv_line = EclPrototype("void ecl_sum_fwrite_interp_csv_line(ecl_sum, time_t, ecl_sum_vector, FILE)", bind=False)
EclSum._get_interp_vector = EclPrototype("void ecl_sum_get_interp_vector(ecl_sum, time_t, ecl_sum_vector, double_vector)", bind=False)
EclSum._init_pandas_frame = EclPrototype("void ecl_sum_init_double_frame(ecl_sum, ecl_sum_vector, double*)", bind=False)
EclSum._init_pandas_frame_interp = EclPrototype("void ecl_sum_init_double_frame_interp(ecl_sum, ecl_sum_vector, time_t_vector, double*)", bind=False)

monkey_the_camel(EclSum, 'varType', EclSum.var_type, classmethod)
monkey_the_camel(EclSum, 'addVariable', EclSum.add_variable)
monkey_the_camel(EclSum, 'addTStep', EclSum.add_t_step)
monkey_the_camel(EclSum, 'assertKeyValid', EclSum.assert_key_valid)
monkey_the_camel(EclSum, 'scaleVector', EclSum.scale_vector)
monkey_the_camel(EclSum, 'shiftVector', EclSum.shift_vector)
monkey_the_camel(EclSum, 'timeRange', EclSum.time_range)
monkey_the_camel(EclSum, 'blockedProduction', EclSum.blocked_production)
monkey_the_camel(EclSum, 'getDataStartTime', EclSum.get_data_start_time)
monkey_the_camel(EclSum, 'getStartTime', EclSum.get_start_time)
monkey_the_camel(EclSum, 'getEndTime', EclSum.get_end_time)
monkey_the_camel(EclSum, 'solveDates', EclSum.solve_dates)
monkey_the_camel(EclSum, 'solveDays', EclSum.solve_days)
monkey_the_camel(EclSum, 'dumpCSVLine', EclSum.dump_csv_line)
monkey_the_camel(EclSum, 'exportCSV', EclSum.export_csv)

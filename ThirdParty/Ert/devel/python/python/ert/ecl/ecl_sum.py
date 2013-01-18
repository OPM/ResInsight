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

# Observe that there is some convention conflict with the C code
# regarding order of arguments: The C code generally takes the time
# index as the first argument and the key/key_index as second
# argument. In the python code this order has been reversed.



import libecl
from   ert.cwrap.cwrap       import *
from   ert.cwrap.cclass      import CClass
from   ert.util.stringlist   import StringList
from   ert.util.ctime        import ctime 

import  numpy


#import ert.ecl_plot.sum_plot as sum_plot

# The date2num function is a verbatim copy of the _to_ordinalf()
# function from the matplotlib.dates module. Inserted here only to
# avoid importing the full matplotlib library. The date2num
# implementation could be replaced with:
#
#   from matplotlib.dates import date2num

HOURS_PER_DAY     = 24.0
MINUTES_PER_DAY   =  60*HOURS_PER_DAY
SECONDS_PER_DAY   =  60*MINUTES_PER_DAY
MUSECONDS_PER_DAY = 1e6*SECONDS_PER_DAY

def date2num( dt ):
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
                 dt.microsecond/MUSECONDS_PER_DAY )
    return base



class EclSumNode:
    def __init__(self , mini_step , report_step , days , date , mpl_date , value):
        """
        EclSumNode is a 'struct' with a summary value and time.

        EclSumNode - a small 'struct' with a summary value and time in
        several formats. When iterating over a EclSumVector instance
        you will get EclSumNode instances. The content of the
        EclSumNode type is stored as plain attributes:

            value       : The actual value
            report_step : The report step
            mini_step   : The ministep
            days        : Days since simulation start
            date        : The simulation date
            mpl_date    : A date format suitable for matplotlib 

        """
        self.value       = value  
        self.report_step = report_step
        self.mini_step   = mini_step
        self.days        = days
        self.date        = date
        self.mpl_date    = mpl_date

    def __str__(self):
        return "<EclSumNode: days:%g  value:%g>" % ( self.days , self.value )


class EclSumVector:
    def __init__(self , parent , key , report_only):
        """
        A summary vector with a vector of values and time.
        
        A summary vector contains the the full time history of one
        key, along with the corresponding time vectors in several
        different time formats. Depending on the report_only argument
        the data vectors in the EclSumVector can either contain all
        the time values, or only those corresponding to report_steps.

        The EclSumVector contains a reference to the parent EclSum
        structure and this is used to implement several of the
        properties and methods of the object; the EclSum vector
        instances should therefor only be instantiated through the
        EclSum.get_vector() method, and not manually with the
        EclSumVector() constructor.
        """
        self.parent         = parent
        self.key            = key
        self.report_only    = report_only
        
        self.__dates       = parent.get_dates( report_only )
        self.__days        = parent.get_days( report_only )
        self.__mpl_dates   = parent.get_mpl_dates( report_only )
        self.__mini_step   = parent.get_mini_step( report_only )
        self.__report_step = parent.get_report_step( report_only )
        self.__values      = None


    def __str__(self):
        return "<Summary vector: %s>" % self.key
        

    @property
    def unit( self ):
        """
        The unit of this vector.
        """
        return self.parent.get_unit( self.key )
    
    def assert_values( self ):
        """
        This function will load and internalize all the values.
        """
        if self.__values == None:
            self.__values = self.parent.get_values( self.key , self.report_only )
            
    @property
    def values( self ):
        """
        All the summary values of the vector, as a numpy vector.
        """
        self.assert_values( )
        return self.__values

    @property
    def dates( self ):
        """
        All the dates of the vector, list of datetime() instances.
        """
        return self.__dates

    @property
    def days( self ):
        """
        The time in days as a numpy vector.

        In the case of lab unit this will be hours.
        """
        return self.__days

    @property
    def mpl_dates( self ):
        """
        All the dates as numpy vector of dates in matplotlib format.
        """
        return self.__mpl_dates

    @property
    def mini_step( self ):
        """
        All the ministeps of the vector.

        Ministeps is the ECLIPSE notion of timesteps. The ministeps
        are numbered sequentially starting at zero; if you have loaded
        the entire simulation the ministep number will correspond to
        the natural indexing. The length of each ministep is
        determined by the convergence properties of the ECLIPSE
        simulation.
        """
        return self.__mini_step

    @property
    def report_step( self ):
        """
        All the report_step of the vector.
        """
        return self.__report_step

    
    def __iget__( self , index ):
        """
        Will return an EclSumNode for element @index; should be called
        through the [] operator, otherwise you can come across
        unitialized data.
        """
        return EclSumNode( self.__mini_step[index] , 
                           self.__report_step[index] , 
                           self.__days[index] , 
                           self.__dates[index] , 
                           self.__mpl_dates[index] , 
                           self.__values[index])
    

    def __len__(self):
        """
        The length of the vector - used for the len() builtin.
        """
        return len(self.__days)


    def __getitem__(self , index):
        """
        Implements the [] operator.
        
        Will return EclSumNode instance according to @index. The index
        value will be interpreted as in a normal python [] lookup,
        i.e. negative values will be interpreted as starting from the
        right and also slice notation is allowed[*]. 

        [*] Observe that in the case of slices the return value will
            not be a proper EclSumVector instance, but rather a normal
            Python list of EclSumNode instances.
        """
        self.assert_values( )
        length = len( self.values )
        if isinstance( index, int):
            if index < 0:
                index += len(self.__values)
            if index < 0 or index > length:
                raise KeyError("Invalid index:%d out of range [0:%d)" % ( index , length))
            else:
                return self.__iget__( index )
        elif isinstance( index , slice ):
            # Observe that the slice based lookup does __not__ return 
            # a proper EclSumVector instance; it will merely return 
            # a simple Python list with EclSumNode instances.
            (start , stop , step) = index.indices( length )
            index = start
            sub_vector = []
            while index < stop:
                sub_vector.append( self.__iget__(index) )
                index += step
            return sub_vector
                
        raise KeyError("Invalid index:%s - must have integer or slice." % index)

    @property
    def first( self ):
        """
        Will return the first EclSumNode in this vector.
        """
        self.assert_values( )
        return self.__iget__( 0 )

    @property
    def last( self ):
        """
        Will return the last EclSumNode in this vector.
        """
        self.assert_values( )
        
        index = len(self.__values) - 1
        return self.__iget__( index )

    @property
    def last_value( self ):
        """
        Will return the last value in this vector.
        """
        self.assert_values( )
        
        index = len(self.__values) - 1
        return self.__iget__( index ).value

    
    
    def get_interp( self , days = None , date = None):
        """
        Will lookup value interpolated to @days or @date.

        The function requires one, and only one, time indicator in
        terms of @days or @date. If the @date variable is given that
        should be Python datetime instance.

           vec = sum["WWCT:A-3"]
           vec.get_interp( days = 100 )
           vec.get_interp( date = datetime.date( year , month , day ))

        This function will crash and burn if the time arguments are
        invalid; if in doubt you should check first.
        """
        return self.parent.get_interp( self.key , days , date )


    def get_interp_vector( self , days_list = None , date_list = None):
        """
        Will return Python list of interpolated values.
        
        See get_interp() for further details.
        """
        return self.parent.get_interp_vector( self.key , days_list , date_list )


    def get_from_report( self , report_step ):
        """
        Will lookup the value based on @report_step.
        """
        return self.parent.get_from_report( self.key , report_step )
    
    #################################################################

    def first_gt_index( self , limit ):
        """
        Locates first index where the value is above @limit.

        Observe that this method will raise an exception if it is
        called from a vector instance with report_only = True.
        """
        if not self.report_only:
            key_index  = cfunc.get_general_var_index( self.parent , self.key )
            time_index = cfunc.get_first_gt( self.parent , key_index , limit )
            return time_index
        else:
            raise Exception("Sorry - first_gt_index() can not be called for vectors with report_only=True")

    def first_gt( self , limit ):
        """
        Locate the first EclSumNode where value is above @limit.

           vec = sum["WWCT:A-3"]
           w = vec.first_gt( 0.50 )
           print "Water cut above 0.50 in well A-3 at: %s" % w.date

        Uses first_gt_index() internally and can not be called for
        vectors with report_only = True.
        """
        time_index = self.first_gt_index( limit )
        print time_index
        if time_index >= 0:
            return self.__iget__( time_index )
        else:
            return None

    def first_lt_index( self , limit ):
        """
        Locates first index where the value is below @limit.

        See first_gt_index() for further details.
        """
        if not self.report_only:
            key_index  = cfunc.get_general_var_index( self.parent , self.key )
            time_index = cfunc.get_first_lt( self.parent , key_index , limit )
            return time_index
        else:
            raise Exception("Sorry - first_lt_index() can not be called for vectors with report_only=True")

    def first_lt( self , limit ):
        """
        Locates first element where the value is below @limit.

        See first_gt() for further details.
        """
        time_index = self.first_lt_index( limit )
        if time_index >= 0:
            return self.__iget__( time_index )
        else:
            return None
            

    #def plot(self):
    #    sum_plot.plot_vector( self )
        



    #################################################################

class EclSum( CClass ):
    
    def __new__( cls , load_case , join_string = ":" , include_restart = True):
        """
        Loads a new EclSum instance with summary data.

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
        """
        c_ptr = cfunc.fread_alloc( load_case , join_string , include_restart)
        if c_ptr:
            obj = object.__new__( cls )
            obj.init_cobj( c_ptr , cfunc.free )
            return obj
        else:
            return None
        

    def __init__(self , load_case , join_string = ":" ,include_restart = True , c_ptr = None):
        """
        Initialize a new EclSum instance.

        See __new__() for further documentation.
        """
        self.load_case       = load_case
        self.join_string     = join_string
        self.include_restart = include_restart
        
        # Initializing the time vectors
        length = self.length
        self.__dates       = [ 0 ] * length
        self.__report_step = numpy.zeros( length , dtype = numpy.int32)
        self.__mini_step   = numpy.zeros( length , dtype = numpy.int32)
        self.__days        = numpy.zeros( length )
        self.__mpl_dates   = numpy.zeros( length )
        for i in range( length ):
            self.__days[i]        = cfunc.iget_sim_days( self , i )
            self.__dates[i]       = cfunc.iget_sim_time( self , i).datetime()
            self.__report_step[i] = cfunc.iget_report_step( self , i )
            self.__mini_step[i]   = cfunc.iget_mini_step( self , i )
            self.__mpl_dates[i]   = date2num( self.__dates[i] )

        index_list = self.report_index_list()
        length = len( index_list )
        self.__datesR       = [ 0 ] * length
        self.__report_stepR = numpy.zeros( length , dtype = numpy.int32)
        self.__mini_stepR   = numpy.zeros( length , dtype = numpy.int32)
        self.__daysR        = numpy.zeros( length )
        self.__mpl_datesR   = numpy.zeros( length )
        for i in range( length ):
            time_index = index_list[ i ]
            self.__daysR[i]        = cfunc.iget_sim_days( self , time_index )
            self.__datesR[i]       = cfunc.iget_sim_time( self , time_index).datetime()
            self.__report_stepR[i] = cfunc.iget_report_step( self , time_index )
            self.__mini_stepR[i]   = cfunc.iget_mini_step( self , time_index )
            self.__mpl_datesR[i]   = date2num( self.__datesR[i] )
        


    def get_vector( self , key , report_only = False):
        """
        Will return EclSumVector according to @key.

        Will raise exception KeyError if the summary object does not
        have @key.
        """
        if self.has_key( key ):
            return EclSumVector( self , key , report_only )
        else:
            raise KeyError("Summary object does not have key: %s" % key)


    def report_index_list( self ):
        """
        Internal function for working with report_steps.
        """
        first_report = self.first_report
        last_report  = self.last_report
        index_list = []
        for report_step in range( first_report , last_report + 1):
            time_index = cfunc.get_report_end( self , report_step )
            index_list.append( time_index )
        return index_list


    def wells( self , pattern = None ):
        """
        Will return a list of all the well names in case.

        If the pattern variable is different from None only wells
        matching the pattern will be returned; the matching is based
        on fnmatch(), i.e. shell style wildcards.
        """
        c_ptr = cfunc.create_well_list( self , pattern )
        return StringList( c_ptr = c_ptr )


    def groups( self , pattern = None ):
        """
        Will return a list of all the group names in case.

        If the pattern variable is different from None only groups
        matching the pattern will be returned; the matching is based
        on fnmatch(), i.e. shell style wildcards.
        """
        c_ptr = cfunc.create_group_list( self , pattern )
        return StringList( c_ptr = c_ptr )

    
    def get_values( self , key , report_only = False):
        """
        Will return numpy vector of all values according to @key.
        
        If the optional argument report_only is true only the values
        corresponding to report steps are included.  The method is
        also available as the 'values' property of an EclSumVector
        instance.
        """
        if self.has_key( key ):
            key_index = cfunc.get_general_var_index( self , key )
            if report_only:
                index_list = self.report_index_list()
                values = numpy.zeros( len(index_list) )
                for i in range(len( index_list)):
                    time_index = index_list[i]
                    values[i]  = cfunc.iiget( self , time_index , key_index )
            else:
                length = cfunc.data_length( self )
                values = numpy.zeros( length )
                for i in range( length ):
                    values[i] = cfunc.iiget( self , i , key_index )
                    
            return values
        else:
            raise KeyError("Summary object does not have key:%s" % key)


    def get_key_index( self , key ):
        """
        Lookup parameter index of @key.

        All the summary keys identified in the SMSPEC file have a
        corresponding index which is used internally. This function
        will return that index for input key @key, this can then be
        used in subsequent calls to e.g. the iiget() method. This is a
        minor optimization in the case of many lookups of the same
        key:

           sum = ecl.EclSum( case )
           key_index = sum.get_key_index( key )
           for time_index in range( sum.length ):
               value = sum.iiget( time_index , key_index )

        Quite low-level function, should probably rather use a
        EclSumVector based function?
        """
        index = cfunc.get_general_var_index( self , key )
        if index >= 0:
            return index
        else:
            return None


    def get_last_value( self , key ):
        """
        Will return the last value corresponding to @key.

        Typically useful to get the total production at end of
        simulation:

           total_production = sum.last_value("FOPT")

        The alternative method 'last' will return a EclSumNode
        instance with some extra time related information.
        """
        return self[key].last_value

    def get_last( self , key ):
        """
        Will return the last EclSumNode corresponding to @key.

        If you are only interested in the final value, you can use the
        get_last_value() method.
        """
        return self[key].last


    def iiget(self , key_index , time_index):
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
        return cfunc.iiget( self , time_index , key_index )


    def iget(self , key , time_index):
        """
        Lookup summary value based on @time_index and key.
        
        The @time_index value should be an integer [0,num_steps) and
        @key should be string key. To get all the water cut values
        from a well:

            for time_index in range( sum.length ):
                wwct = sum.iget( time_index , "WWCT:W5" )
                
        This is a quite low level function, in most cases it will be
        natural to go via e.g. an EclSumVector instance.
        """
        return cfunc.get_general_var( self , time_index , key )


    def __getitem__(self , key):
        """
        Implements [] operator - @key should be a summary key.

        The returned value will be a EclSumVector instance.
        """
        return self.get_vector( key )

    
    def get_interp( self , key , days = None , date = None):
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
        if days:
            if date:
                raise ValueError("Must supply either days or date")
            else:
                if cfunc.check_sim_days( self , days ):
                    return cfunc.get_general_var_from_sim_days( self , days , key )
                else:
                    raise ValueError("days:%s is outside range of simulation: [%g,%g]" % (days , self.first_day , self.sim_length))
        elif date:
            if cfunc.check_sim_time( self , ctime(date) ):
                return cfunc.get_general_var_from_sim_time( self , ctime(date) , key )
            else:
                raise ValueError("date:%s is outside range of simulation data" % date)
        else:
            raise ValueError("Must supply either days or date")


    def get_report( self , date = None , days = None):
        """
        Will return the report step corresponding to input @date or @days.

        If the input argument does not correspond to any report steps
        the function will return -1. Observe that the function
        requires strict equality.
        """
        if date:
            if days:
                raise ValueError("Must supply either days or date")
            step = cfunc.get_report_step_from_time( self , ctime(date))
        elif days:
            step = cfunc.get_report_step_from_days( self , days)
            
        return step


    def get_report_time( self , report):
        """
        Will return the datetime corresponding to the report_step @report.
        """
        ctime = cfunc.get_report_time( self , report )
        return ctime.date()


    def get_interp_vector( self , key , days_list = None , date_list = None):
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
        if days_list:
            if date_list:
                raise ValueError("Must supply either days_list or date_list")
            else:
                vector = numpy.zeros( len(days_list ))
                sim_length = self.sim_length
                sim_start  = self.first_day
                index = 0
                for days in days_list:
                    if days >= sim_start and days < sim_length:
                        vector[index] = cfunc.get_general_var_from_sim_days( self , days , key)
                    else:
                        raise ValueError("Invalid days value")
                    index += 1
        elif date_list:
            start_time = self.data_start
            end_time   = self.end_date
            vector = numpy.zeros( len(date_list ))
            index = 0
            for date in date_list:
                if date >= start_time and date <= end_time:
                    vector[index] =  cfunc.get_general_var_from_sim_time( self , ctime(date) , key) 
                else:
                    raise ValueError("Invalid date value")
                index += 1
        else:
            raise ValueError("Must supply either days_list or date_list")
        return vector
                
    

    def get_from_report( self , key , report_step ):
        """
        Return summary value of @key at time @report_step.
        """
        time_index = cfunc.get_report_end( self , report_step )
        return cfunc.get_general_var( self , time_index , key )
    

    def has_key( self , key):
        """
        Check if summary object has key @key.
        """
        return cfunc.has_key( self, key )

    def unit(self , key):
        """
        Will return the unit of @key. 
        """
        return cfunc.get_unit( self , key )

    @property
    def case(self):
        """
        Will return the case name of the current instance - optionally including path.
        """
        return cfunc.get_simcase( self )


    @property
    def path(self):
        """
        Will return the path to the current case. Will be None for 
        case in CWD. See also abs_path.
        """
        return cfunc.get_path( self )

    @property
    def base(self):
        """
        Will return the basename of the current case - no path.
        """
        return cfunc.get_base( self )

    @property
    def abs_path(self):
        """
        Will return the absolute path to the current case.
        """
        return cfunc.get_abs_path( self )

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
    def days( self ):
        """
        Will return a numpy vector of simulations days.
        """
        return self.get_days(  False )

    def get_days( self , report_only = False):
        """
        Will return a numpy vector of simulations days.

        If the optional argument @report_only is set to True, only
        'days' values corresponding to report steps will be included.
        """
        if report_only:
            return self.__daysR
        else:
            return self.__days

    @property 
    def dates( self ):
        """
        Will return a list of simulation dates.

        The list will be an ordinary Python list, and the dates will
        be in terms ordinary Python datetime values.
        """
        return self.get_dates( False )

    def get_dates( self , report_only = False):
        """
        Will return a list of simulation dates.

        The list will be an ordinary Python list, and the dates will
        be in terms ordinary Python datetime values. If the optional
        argument @report_only is set to True, only dates corresponding
        to report steps will be included.
        """
        if report_only:
            return self.__datesR
        else:
            return self.__dates

    @property
    def mpl_dates( self ):
        """
        Will return a numpy vector of dates ready for matplotlib

        The content of the vector are dates in matplotlib format,
        i.e. floats - generated by the date2num() function at the top
        of this file.
        """
        return self.get_mpl_dates( False )

    def get_mpl_dates( self , report_only = False):
        """
        Will return a numpy vector of dates ready for matplotlib

        If the optional argument @report_only is set to True, only
        dates values corresponding to report steps will be
        included. The content of the vector are dates in matplotlib
        format, i.e. floats - generated by the date2num() function at
        the top of this file.
        """
        if report_only:
            return self.__mpl_datesR
        else:
            return self.__mpl_dates
    
    @property 
    def mini_step( self ):
        """
        Will return a a python list of ministep values.

        Will return a Python list of ministep values from this summary
        case; the ministep values are the internal indexing of
        timesteps provided by the reservoir simulator. In normal cases
        this will be: [0,1,2,3,4,5,....], but in the case of restarted
        simulations it can start at a higher value, and there can also
        be 'holes' in the series if 'RPTONLY' has been used in THE
        ECLIPSE datafile.
        """
        return self.get_mini_step( False )

    def get_mini_step( self , report_only = False):
        """
        Will return a a python list of ministep values.
        
        If the optional argument @report_only is set to True, only
        dates values corresponding to report steps will be
        included. See documentation of property: 'mini_step' for
        further documentation.
        """
        if report_only:
            return self.__mini_stepR
        else:
            return self.__mini_step


    @property 
    def report_step( self ):
        """
        Will return a list of report steps.

        The simulator will typically use several simulation timesteps
        for each report step, and the number will change between
        different report steps. So - assuming that the first report
        step one has five simulations timesteps and the next two have
        three the report_step vector can look like:

          [...,1,1,1,1,1,2,2,2,3,3,3,....]
            
        """
        return self.get_report_step( False )

    def get_report_step( self , report_only = False):
        if report_only:
            return self.__report_stepR
        else:
            return self.__report_step

    #-----------------------------------------------------------------        
        
    def iget_days(self , time_index):
        """
        Returns the number of simulation days for element nr @time_index.
        """
        return cfunc.iget_sim_days( self , time_index )

    def iget_date(self , time_index):
        """
        Returns the simulation date for element nr @time_index.
        """
        return cfunc.iget_sim_time( self , time_index ).datetime()

    def iget_report( self , time_index ):
        """
        Returns the report step corresponding to @time_index.
        
        One report step will in general contain many ministeps.
        """
        return cfunc.iget_report_step( self , time_index )
    

    @property
    def length(self):
        """
        The number of timesteps in the dataset.
        """
        return cfunc.data_length( self )

    @property
    def first_day(self):
        """
        The first day we have simulation data for; normally 0.
        """
        return cfunc.get_first_day( self )

    @property 
    def sim_length( self ):
        """
        The length of the current dataset in simulation days.

        Will include the length of a leading restart section,
        irrespective of whether we have data for this or not.
        """
        return cfunc.sim_length( self )

    @property
    def start_date(self):
        """
        A Python date instance with the start date.

        The start time is taken from the SMSPEC file, and in case not
        all timesteps have been loaded, e.g. for a restarted case, the
        returned start_date might be different from the datetime of
        the first (loaded) timestep.
        """
        ctime = cfunc.get_start_date( self ) 
        return ctime.date()

    @property
    def end_date(self):
        """
        The date of the last (loaded) time step.
        """
        ctime = cfunc.get_end_date( self )
        return ctime.date()

    @property
    def data_start(self):
        """
        The first date we have data for.
        """
        ctime = cfunc.get_data_start( self )
        return ctime.date()
    

    @property
    def start_time(self):
        """
        A Python datetime instance with the start time.
        
        See start_date() for further details.
        """
        ctime = cfunc.get_start_date( self ) 
        return ctime.datetime()

    @property
    def end_time(self):
        """
        The time of the last (loaded) time step.
        """
        ctime = cfunc.get_end_date( self )
        return ctime.datetime()
    
    @property
    def last_report(self):
        """
        The number of the last report step in the dataset.
        """
        return cfunc.get_last_report_step( self )

    @property
    def first_report(self):
        """
        The number of the last report step in the dataset.
        """
        return cfunc.get_first_report_step( self )

    def first_gt_index( self , key , limit ):
        """
        Returns the first index where @key is above @limit.
        """
        key_index  = cfunc.get_general_var_index( self , key )
        time_index = cfunc.get_first_lt( self , key_index , limit )
        return time_index

    def first_lt_index( self , key , limit ):
        """
        Returns the first index where @key is below @limit.
        """
        key_index  = cfunc.get_general_var_index( self , key )
        time_index = cfunc.get_first_lt( self , key_index , limit )
        return time_index

    def first_gt( self , key , limit ):
        """
        First EclSumNode of @key which is above @limit.
        """
        vector = self[key]
        return vector.first_gt( limit )
    
    def first_lt(self , key , limit ):
        """
        First EclSumNode of @key which is below @limit.
        """
        vector = self[key]
        return vector.first_lt( limit )


    def keys( self , pattern = None):
        """
        Return a list of summary keys matching @pattern.
        
        The matching algorithm is ultimately based on the fnmatch()
        function, i.e. normal shell-character syntax is used. With
        @pattern == "WWCT:*" you will get a list of watercut keys for
        all wells. 
        
        If pattern is None you will get all the keys of summary
        object.
        """
        s = StringList()
        cfunc.select_matching_keys( self , pattern , s )
        return s.strings


    def fwrite(self , ecl_case = None):
        if ecl_case:
            cfunc.set_case( self , ecl_case )
            
        cfunc.fwrite_sum( self )
        
        
        



#################################################################

# 2. Creating a wrapper object around the libecl library, 
#    registering the type map : ecl_kw <-> EclKW
cwrapper = CWrapper( libecl.lib )
cwrapper.registerType( "ecl_sum" , EclSum )


# 3. Installing the c-functions used to manipulate ecl_kw instances.
#    These functions are used when implementing the EclKW class, not
#    used outside this scope.
cfunc = CWrapperNameSpace("ecl_sum")

cfunc.create_well_list              = cwrapper.prototype("c_void_p ecl_sum_alloc_well_list( ecl_sum , char* )")
cfunc.create_group_list             = cwrapper.prototype("c_void_p ecl_sum_alloc_group_list( ecl_sum , char* )")
cfunc.fread_alloc                   = cwrapper.prototype("c_void_p ecl_sum_fread_alloc_case__( char* , char* , bool)") 
cfunc.iiget                         = cwrapper.prototype("double   ecl_sum_iget( ecl_sum , int , int)")
cfunc.free                          = cwrapper.prototype("void     ecl_sum_free( ecl_sum )")
cfunc.data_length                   = cwrapper.prototype("int      ecl_sum_get_data_length( ecl_sum )")
cfunc.iget_sim_days                 = cwrapper.prototype("double   ecl_sum_iget_sim_days( ecl_sum , int) ")
cfunc.iget_report_step              = cwrapper.prototype("int      ecl_sum_iget_report_step( ecl_sum , int) ")
cfunc.iget_mini_step                = cwrapper.prototype("int      ecl_sum_iget_mini_step( ecl_sum , int) ")
cfunc.iget_sim_time                 = cwrapper.prototype("time_t   ecl_sum_iget_sim_time( ecl_sum , int) ")
cfunc.get_report_end                = cwrapper.prototype("int      ecl_sum_iget_report_end( ecl_sum , int)")
cfunc.get_general_var               = cwrapper.prototype("double   ecl_sum_get_general_var( ecl_sum , int , char*)")
cfunc.get_general_var_index         = cwrapper.prototype("int      ecl_sum_get_general_var_params_index( ecl_sum , char*)")
cfunc.get_general_var_from_sim_days = cwrapper.prototype("double   ecl_sum_get_general_var_from_sim_days( ecl_sum , double , char*)")
cfunc.get_general_var_from_sim_time = cwrapper.prototype("double   ecl_sum_get_general_var_from_sim_time( ecl_sum , time_t , char*)")
cfunc.get_first_gt                  = cwrapper.prototype("int      ecl_sum_get_first_gt( ecl_sum , int , double )")
cfunc.get_first_lt                  = cwrapper.prototype("int      ecl_sum_get_first_lt( ecl_sum , int , double )")
cfunc.get_start_date                = cwrapper.prototype("time_t   ecl_sum_get_start_time( ecl_sum )")
cfunc.get_end_date                  = cwrapper.prototype("time_t   ecl_sum_get_end_time( ecl_sum )")
cfunc.get_last_report_step          = cwrapper.prototype("int      ecl_sum_get_last_report_step( ecl_sum )")
cfunc.get_first_report_step         = cwrapper.prototype("int      ecl_sum_get_first_report_step( ecl_sum )")
cfunc.iget_report_step              = cwrapper.prototype("int      ecl_sum_iget_report_step( ecl_sum , int )")
cfunc.select_matching_keys          = cwrapper.prototype("void     ecl_sum_select_matching_general_var_list( ecl_sum , char* , stringlist )")
cfunc.has_key                       = cwrapper.prototype("bool     ecl_sum_has_general_var( ecl_sum , char* )")
cfunc.check_sim_time                = cwrapper.prototype("bool     ecl_sum_check_sim_time( ecl_sum , time_t )") 
cfunc.check_sim_days                = cwrapper.prototype("bool     ecl_sum_check_sim_days( ecl_sum , double )") 
cfunc.sim_length                    = cwrapper.prototype("double   ecl_sum_get_sim_length( ecl_sum )")
cfunc.get_first_day                 = cwrapper.prototype("double   ecl_sum_get_first_day( ecl_sum )")
cfunc.get_data_start                = cwrapper.prototype("time_t   ecl_sum_get_data_start( ecl_sum )")

cfunc.get_unit                      = cwrapper.prototype("char*    ecl_sum_get_unit( ecl_sum , char*)") 
cfunc.get_simcase                   = cwrapper.prototype("char*    ecl_sum_get_case( ecl_sum )")
cfunc.get_base                      = cwrapper.prototype("char*    ecl_sum_get_base( ecl_sum )")
cfunc.get_path                      = cwrapper.prototype("char*    ecl_sum_get_path( ecl_sum )")
cfunc.get_abs_path                  = cwrapper.prototype("char*    ecl_sum_get_abs_path( ecl_sum )")
cfunc.get_report_step_from_time     = cwrapper.prototype("int      ecl_sum_get_report_step_from_time( ecl_sum , time_t)")
cfunc.get_report_step_from_days     = cwrapper.prototype("int      ecl_sum_get_report_step_from_days( ecl_sum , double)")
cfunc.get_report_time               = cwrapper.prototype("time_t   ecl_sum_get_report_time(ecl_sum , int)")

cfunc.fwrite_sum                    = cwrapper.prototype("void     ecl_sum_fwrite(ecl_sum)")
cfunc.set_case                      = cwrapper.prototype("void     ecl_sum_set_case(ecl_sum, char*)")

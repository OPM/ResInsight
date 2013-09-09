from ert.ecl.ecl_sum_node import EclSumNode


class EclSumVector:
    def __init__(self, parent, key, report_only):
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
        self.parent = parent
        self.key = key
        self.report_only = report_only

        self.__dates = parent.get_dates(report_only)
        self.__days = parent.get_days(report_only)
        self.__mpl_dates = parent.get_mpl_dates(report_only)
        self.__mini_step = parent.get_mini_step(report_only)
        self.__report_step = parent.get_report_step(report_only)
        self.__values = None


    def __str__(self):
        return "<Summary vector: %s>" % self.key


    @property
    def unit( self ):
        """
        The unit of this vector.
        """
        return self.parent.get_unit(self.key)

    def assert_values( self ):
        """
        This function will load and internalize all the values.
        """
        if self.__values is None:
            self.__values = self.parent.get_values(self.key, self.report_only)

    @property
    def values( self ):
        """
        All the summary values of the vector, as a numpy vector.
        """
        self.assert_values()
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


    def __iget__( self, index ):
        """
        Will return an EclSumNode for element @index; should be called
        through the [] operator, otherwise you can come across
        unitialized data.
        """
        return EclSumNode(self.__mini_step[index],
                          self.__report_step[index],
                          self.__days[index],
                          self.__dates[index],
                          self.__mpl_dates[index],
                          self.__values[index])


    def __len__(self):
        """
        The length of the vector - used for the len() builtin.
        """
        return len(self.__days)


    def __getitem__(self, index):
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
        self.assert_values()
        length = len(self.values)
        if isinstance(index, int):
            if index < 0:
                index += len(self.__values)
            if index < 0 or index > length:
                raise KeyError("Invalid index:%d out of range [0:%d)" % ( index, length))
            else:
                return self.__iget__(index)
        elif isinstance(index, slice):
            # Observe that the slice based lookup does __not__ return
            # a proper EclSumVector instance; it will merely return
            # a simple Python list with EclSumNode instances.
            (start, stop, step) = index.indices(length)
            index = start
            sub_vector = []
            while index < stop:
                sub_vector.append(self.__iget__(index))
                index += step
            return sub_vector

        raise KeyError("Invalid index:%s - must have integer or slice." % index)

    @property
    def first( self ):
        """
        Will return the first EclSumNode in this vector.
        """
        self.assert_values()
        return self.__iget__(0)

    @property
    def last( self ):
        """
        Will return the last EclSumNode in this vector.
        """
        self.assert_values()

        index = len(self.__values) - 1
        return self.__iget__(index)

    @property
    def last_value( self ):
        """
        Will return the last value in this vector.
        """
        self.assert_values()

        index = len(self.__values) - 1
        return self.__iget__(index).value


    def get_interp( self, days=None, date=None):
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
        return self.parent.get_interp(self.key, days, date)


    def get_interp_vector( self, days_list=None, date_list=None):
        """
        Will return Python list of interpolated values.

        See get_interp() for further details.
        """
        return self.parent.get_interp_vector(self.key, days_list, date_list)


    def get_from_report( self, report_step ):
        """
        Will lookup the value based on @report_step.
        """
        return self.parent.get_from_report(self.key, report_step)

    #################################################################

    def first_gt_index( self, limit ):
        """
        Locates first index where the value is above @limit.

        Observe that this method will raise an exception if it is
        called from a vector instance with report_only = True.
        """
        if not self.report_only:
            key_index = self.parent.cNamespace().get_general_var_index(self.parent, self.key)
            time_index = self.parent.cNamespace().get_first_gt(self.parent, key_index, limit)
            return time_index
        else:
            raise Exception("Sorry - first_gt_index() can not be called for vectors with report_only=True")

    def first_gt( self, limit ):
        """
        Locate the first EclSumNode where value is above @limit.

           vec = sum["WWCT:A-3"]
           w = vec.first_gt( 0.50 )
           print "Water cut above 0.50 in well A-3 at: %s" % w.date

        Uses first_gt_index() internally and can not be called for
        vectors with report_only = True.
        """
        time_index = self.first_gt_index(limit)
        print time_index
        if time_index >= 0:
            return self.__iget__(time_index)
        else:
            return None

    def first_lt_index( self, limit ):
        """
        Locates first index where the value is below @limit.

        See first_gt_index() for further details.
        """
        if not self.report_only:
            key_index = self.parent.cNamespace().get_general_var_index(self.parent, self.key)
            time_index = self.parent.cNamespace().get_first_lt(self.parent, key_index, limit)
            return time_index
        else:
            raise Exception("Sorry - first_lt_index() can not be called for vectors with report_only=True")

    def first_lt( self, limit ):
        """
        Locates first element where the value is below @limit.

        See first_gt() for further details.
        """
        time_index = self.first_lt_index(limit)
        if time_index >= 0:
            return self.__iget__(time_index)
        else:
            return None

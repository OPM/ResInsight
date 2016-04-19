import datetime

class limit_property(object):
    def __init__(self, attribute_name, types, minimum=None, maximum=None):
        super(limit_property, self).__init__()
        self._types = types
        self._maximum = maximum
        self._minimum = minimum
        self._attribute_name = attribute_name

    def __get__(self, instance, owner):
        if not hasattr(instance, "_%s" % self._attribute_name):
            setattr(instance, "_%s" % self._attribute_name, None)
        return getattr(instance, "_%s" % self._attribute_name)

    def __set__(self, instance, value):
        if value is not None:
            if not isinstance(value, self._types):
                raise TypeError("Value not (one) of type(s): %s: %s" % (self._types, repr(value)))
            if self._minimum is not None and value < self._minimum:
                raise ValueError("Value can not be less than %f: %f < %f" % (self._minimum, value, self._minimum))
            if self._maximum is not None and value > self._maximum:
                raise ValueError("Value can not be larger than %f: %f > %f" % (self._maximum, value, self._maximum))

        setattr(instance, "_%s" % self._attribute_name, value)


class limits_property(object):
    def __init__(self, minimum_attribute_name, maximum_attribute_name):
        super(limits_property, self).__init__()
        self._minimum_attribute_name = minimum_attribute_name
        self._maximum_attribute_name = maximum_attribute_name

    def __get__(self, instance, owner):
        return getattr(instance, "%s" % self._minimum_attribute_name), getattr(instance, "%s" % self._maximum_attribute_name)

    def __set__(self, instance, value):
        setattr(instance, "_%s" % self._minimum_attribute_name, value[0])
        setattr(instance, "_%s" % self._maximum_attribute_name, value[1])


class PlotLimits(object):
    value_minimum = limit_property("value_minimum", (float, int))
    """ :type: float """
    value_maximum = limit_property("value_maximum", (float, int))
    """ :type: float """
    value_limits = limits_property("value_minimum", "value_maximum")
    """ :type: (float, float) """

    index_minimum = limit_property("index_minimum", int, minimum=0)
    """ :type: int """
    index_maximum = limit_property("index_maximum", int, minimum=0)
    """ :type: int """
    index_limits = limits_property("index_minimum", "index_maximum")
    """ :type: (int, int) """

    count_minimum = limit_property("count_minimum", int, minimum=0)
    """ :type: int """
    count_maximum = limit_property("count_maximum", int, minimum=0)
    """ :type: int """
    count_limits = limits_property("count_minimum", "count_maximum")
    """ :type: (int, int) """

    density_minimum = limit_property("density_minimum", (float, int), minimum=0.0)
    """ :type: float """
    density_maximum = limit_property("density_maximum", (float, int), minimum=0.0)
    """ :type: float """
    density_limits = limits_property("density_minimum", "density_maximum")
    """ :type: (float, float) """

    depth_minimum = limit_property("depth_minimum", (float, int), minimum=0.0)
    """ :type: float """
    depth_maximum = limit_property("depth_maximum", (float, int), minimum=0.0)
    """ :type: float """
    depth_limits = limits_property("depth_minimum", "depth_maximum")
    """ :type: tuple[float, float] """

    date_minimum = limit_property("date_minimum", (datetime.date, datetime.datetime))
    """ :type: datetime.datetime or datetime.date """
    date_maximum = limit_property("date_maximum", (datetime.date, datetime.datetime))
    """ :type: datetime.datetime or datetime.date """
    date_limits = limits_property("date_minimum", "date_maximum")
    """ :type: tuple[datetime.datetime|datetime.date, datetime.datetime|datetime.date] """


    def __eq__(self, other):
        """ @type other: PlotLimits """
        equality = self.value_limits == other.value_limits
        equality = equality and self.index_limits == other.index_limits
        equality = equality and self.count_limits == other.count_limits
        equality = equality and self.depth_limits == other.depth_limits
        equality = equality and self.date_limits == other.date_limits
        equality = equality and self.density_limits == other.density_limits

        return equality


    def copyLimitsFrom(self, other):
        """ @type other: PlotLimits """
        self.value_limits = other.value_limits
        self.density_limits = other.density_limits
        self.depth_limits = other.depth_limits
        self.index_limits = other.index_limits
        self.date_limits = other.date_limits
        self.count_limits = other.count_limits
class EclSumNode(object):

    def __init__(self, report_step, days, date, mpl_date, value):
        """
        EclSumNode is a 'struct' with a summary value and time.

        EclSumNode - a small 'struct' with a summary value and time in
        several formats. When iterating over a EclSumVector instance
        you will get EclSumNode instances. The content of the
        EclSumNode type is stored as plain attributes:

            value       : The actual value
            report_step : The report step
            days        : Days since simulation start
            date        : The simulation date
            mpl_date    : A date format suitable for matplotlib

        """
        self.value = value
        self.report_step = report_step
        self.days = days
        self.date = date
        self.mpl_date = mpl_date

    def __repr__(self):
        return "EclSumNode(days=%d, value=%g)" % (self.days, self.value)

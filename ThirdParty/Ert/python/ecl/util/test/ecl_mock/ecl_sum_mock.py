import datetime
from ecl.summary import EclSum


def mock_func(ecl_sum , key , days):
    return days * 10


def createEclSum( case,
                  keys,
                  sim_start = datetime.date(2010 , 1, 1),
                  data_start = None,
                  sim_length_days = 5 * 365,
                  num_report_step = 5,
                  num_mini_step = 10,
                  dims = (20,10,5) ,
                  func_table = {},
                  restart_case = None,
                  restart_step = -1):

    ecl_sum = EclSum.restart_writer(case , restart_case, restart_step, sim_start , dims[0] , dims[1] , dims[2])
    var_list = []
    for (kw,wgname,num,unit) in keys:
        var_list.append( ecl_sum.addVariable( kw , wgname = wgname , num = num, unit =unit) )

    # This is a bug! This should not be integer division, but tests are written
    # around that assumption.
    report_step_length = float(sim_length_days // num_report_step)
    mini_step_length = float(report_step_length // num_mini_step)

    if data_start is None:
        time_offset = 0
    else:
        dt = data_start - sim_start
        time_offset = dt.total_seconds() / 86400.0

    for report_step in range(num_report_step):
        for mini_step in range(num_mini_step):
            days = time_offset + report_step * report_step_length + mini_step * mini_step_length
            t_step = ecl_sum.addTStep( report_step + 1 , sim_days = days )


            for var in var_list:
                key = var.getKey1( )

                if key in func_table:
                    func = func_table[key]
                    t_step[key] = func( days )
                else:
                    t_step[key] = mock_func( ecl_sum , key , days)

    return ecl_sum

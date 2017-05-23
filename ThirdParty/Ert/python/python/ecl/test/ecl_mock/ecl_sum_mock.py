import datetime
from ecl.ecl import EclSum


def mock_func(ecl_sum , key , days):
    return days * 10


def createEclSum( case , keys , start = datetime.date(2010 , 1, 1) , sim_length_days = 5 * 365 , num_report_step = 5, num_mini_step = 10, dims = (20,10,5) , func_table = {}):
    ecl_sum = EclSum.writer(case , start , dims[0] , dims[1] , dims[2])
    var_list = []
    for (kw,wgname,num) in keys:
        var_list.append( ecl_sum.addVariable( kw , wgname = wgname , num = num) )

    report_step_length = sim_length_days / num_report_step
    mini_step_length = report_step_length / num_mini_step
    for report_step in range(num_report_step):
        for mini_step in range(num_mini_step):
            days = report_step * report_step_length + mini_step * mini_step_length
            t_step = ecl_sum.addTStep( report_step + 1 , sim_days = days )
                
            for var in var_list:
                key = var.getKey1( )
                if key in func_table:
                    func = func_table[key]
                    t_step[key] = func( days )
                else:
                    t_step[key] = mock_func( ecl_sum , key , days)

    return ecl_sum

import math

def run(ecl_state, schedule, report_step, summary_state, actionx_callback):

    wells = []
    if report_step == 1:
        wells = ["P1"]
    elif report_step == 2:
        wells = ["P2"]
    elif report_step == 3:
        wells = ["P3"]
    elif report_step == 4:
        wells = ["P4"]

    actionx_callback("CLOSEWELL", wells)

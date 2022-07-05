import math

def run(ecl_state, schedule, report_step, summary_state, actionx_callback):
    if not "run_count" in summary_state:
        summary_state["run_count"] = 0
    summary_state["run_count"] += 1

    if summary_state.elapsed() > (365 + 15)*24*3600:
        return True

    return False

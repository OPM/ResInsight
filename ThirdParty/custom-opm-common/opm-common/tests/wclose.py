import sys

def run(ecl_state, schedule, report_step, summary_state, actionx_callback):
    sys.stdout.write("Running PYACTION arg1:{}\n".format(ecl_state))
    if "FOPR" in summary_state:
        sys.stdout.write("Have FOPR: {}\n".format( summary_state["FOPR"] ))
    else:
        sys.stdout.write("Missing FOPR\n")

    grid = ecl_state.grid()
    sys.stdout.write("Grid dimensions: ({},{},{})\n".format(grid.nx, grid.ny, grid.nz))

    prod_well = schedule.get_well("PROD1", report_step)
    sys.stdout.write("Well status: {}\n".format(prod_well.status()))
    if not "list" in storage:
        storage["list"] = []
    storage["list"].append(report_step)
    sys.stdout.write("storage[list]: {}\n".format(storage["list"]))

    if summary_state.well_var("PROD1", "WWCT") > 0.80:
        schedule.shut_well("PROD1", report_step)
        schedule.open_well("PROD2", report_step)
        summary_state.update("RUN_COUNT", 1)
    return True

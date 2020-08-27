def run(ecl_state, schedule, report_step, sim):
    wells_shut = False

    for well in sim.wells:
        if sim.well_var(well, "WWCT") > 0.50:
            schedule.shut_well(well, report_step)
            wells_shut = True

    return wells_shut

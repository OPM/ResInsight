import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_10k_well_log_extraction(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    case_path = case_root_path + "/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 2
    well_path_files = [
        case_root_path + "/wellpath_a.dev",
    ]

    view = case.create_view()
    view.set_time_step(1)

    well_path_names = rips_instance.project.import_well_paths(well_path_files)
    wells = rips_instance.project.well_paths()
    well_path = wells[0]

    properties = [
        ("STATIC_NATIVE", "INDEX_K", 0),
        ("STATIC_NATIVE", "PORO", 0),
        ("STATIC_NATIVE", "PERMX", 0),
        ("DYNAMIC_NATIVE", "PRESSURE", 0),
    ]

    well_log_plot_collection = rips_instance.project.descendants(
        rips.WellLogPlotCollection
    )[0]

    well_log_plot = well_log_plot_collection.new_well_log_plot(case, well_path)

    # Create a track for each property
    for prop_type, prop_name, time_step in properties:
        track = well_log_plot.new_well_log_track("Track: " + prop_name, case, well_path)
        c = track.add_extraction_curve(case, well_path, prop_type, prop_name, time_step)

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        well_log_plot.export_data_as_las(export_folder=tmpdirname)

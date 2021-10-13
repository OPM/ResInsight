import sys
import os
import tempfile

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_10k(rips_instance, initialize_test):
    case_root_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC"
    project_path = case_root_path + "/well_completions_pytest.rsp"
    project = rips_instance.project.open(path=project_path)

    export_folder = tempfile.gettempdir()

    rips_instance.set_export_folder(export_type="COMPLETIONS", path=export_folder)

    case = project.cases()[0]
    case.export_well_path_completions(
        time_step=1,
        well_path_names=["Well-1"],
        file_split="UNIFIED_FILE",
    )

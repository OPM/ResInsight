############################################################################
# This script will export completions for a well path for all cases in the project
#
############################################################################

import os
import rips

# Load instance
resinsight = rips.Instance.find()
cases = resinsight.project.cases()

for case in cases:
    print("Case name: ", case.name)
    print("Case id: ", case.id)

    case.export_well_path_completions(
        time_step=0,
        well_path_names=["Well-1"],
        file_split="UNIFIED_FILE",
        include_perforations=True,
        custom_file_name="d:/scratch/well_path_export/myfile.myext",
    )

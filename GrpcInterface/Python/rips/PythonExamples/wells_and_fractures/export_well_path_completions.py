############################################################################
# This script will export completions for a well path for all cases in the project
#
############################################################################

import rips

# Load instance
resinsight = rips.Instance.find()
cases = resinsight.project.cases()
well_path = resinsight.project.well_path_by_name("Well-1")

for case in cases:
    print("Case name: ", case.name)
    print("Case id: ", case.id)

    case.export_completions(
        well_paths=[well_path],
        time_step=0,
        export_folder="d:/scratch/well_path_export",
        file_split=rips.CompletionExportSplit.UNIFIED_FILE,
        include_perforations=True,
        custom_file_name="myfile.myext",
    )

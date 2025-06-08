# Load ResInsight Processing Server Client Library
import rips
import tempfile
from os.path import expanduser
from pathlib import Path

# Connect to ResInsight instance
resinsight = rips.Instance.find()


home_dir = expanduser("~")

export_folder = tempfile.mkdtemp()

directory_path = "resprojects/webviz-subsurface-testdata/reek_history_match/"
# directory_path = "e:/gitroot/webviz-subsurface-testdata/reek_history_match"


case_file_paths = []
num_realizations = 9
num_iterations = 4


for realization in range(0, num_realizations):
    for iteration in range(0, num_iterations):
        realization_dir = "realization-" + str(realization)
        iteration_dir = "iter-" + str(iteration)
        egrid_name = "eclipse/model/5_R001_REEK-" + str(realization) + ".EGRID"
        path = Path(
            home_dir, directory_path, realization_dir, iteration_dir, egrid_name
        )
        case_file_paths.append(path)

k_indexes = [4, 10]

for path in case_file_paths:
    # Load a case
    path_name = path.as_posix()
    case = resinsight.project.load_case(path_name)

    if resinsight.project.has_warnings():
        for warning in resinsight.project.warnings():
            print(warning)

    surface_collection = resinsight.project.descendants(rips.SurfaceCollection)[0]

    for k_index in k_indexes:
        print("Generating surface K layer " + str(k_index) + " for case " + path_name)

        surface = surface_collection.new_surface(case, k_index)
        print("Surface: ", surface)

        parent_path = path.parent
        export_folder_path = Path(parent_path, "surfaceexport")
        export_folder_path.mkdir(parents=True, exist_ok=True)

        export_file = Path(export_folder_path, "surf_" + str(k_index) + ".ts")
        print("Exporting to " + export_file.as_posix())
        surface.export_to_file(export_file.as_posix())

    # Close project to avoid aggregated memory usage
    # Can be replaced when case.close() is implemented
    resinsight.project.close()

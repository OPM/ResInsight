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

command_router = resinsight.command_router

for path in case_file_paths:
    path_name = path.as_posix()

    command_router.extract_surfaces(path_name, k_indexes)

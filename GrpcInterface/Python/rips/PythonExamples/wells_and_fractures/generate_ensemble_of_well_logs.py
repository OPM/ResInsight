# Load ResInsight Processing Server Client Library
import rips
import tempfile
from os.path import expanduser
from pathlib import Path

# Connect to ResInsight instance
resinsight = rips.Instance.find()


home_dir = expanduser("~")


properties = [
    ("STATIC_NATIVE", "INDEX_K", 0),
    ("STATIC_NATIVE", "PORO", 0),
    ("STATIC_NATIVE", "PERMX", 0),
    ("DYNAMIC_NATIVE", "PRESSURE", 0),
]

export_folder = tempfile.mkdtemp()

directory_path = "resprojects/webviz-subsurface-testdata/reek_history_match/"


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

for path in case_file_paths:
    # Load a case
    path_name = path.as_posix()
    grid_only = True
    case = resinsight.project.load_case(path_name, grid_only)

    # Load some wells
    well_paths = resinsight.project.import_well_paths(
        well_path_files=[
            Path(home_dir, directory_path, "wellpaths", "Well-1.dev").as_posix(),
            Path(home_dir, directory_path, "wellpaths", "Well-2.dev").as_posix(),
        ]
    )

    if resinsight.project.has_warnings():
        for warning in resinsight.project.warnings():
            print(warning)

    well_log_plot_collection = resinsight.project.descendants(
        rips.WellLogPlotCollection
    )[0]

    for well_path in well_paths:
        print(
            "Generating las file for well: " + well_path.name + " in case: " + path_name
        )

        well_log_plot = well_log_plot_collection.new_well_log_plot(case, well_path)

        # Create a track for each property
        for prop_type, prop_name, time_step in properties:
            track = well_log_plot.new_well_log_track(
                "Track: " + prop_name, case, well_path
            )

            c = track.add_extraction_curve(
                case, well_path, prop_type, prop_name, time_step
            )

        parent_path = path.parent
        export_folder_path = Path(parent_path, "lasexport")
        export_folder_path.mkdir(parents=True, exist_ok=True)

        export_folder = export_folder_path.as_posix()
        well_log_plot.export_data_as_las(export_folder=export_folder)

    resinsight.project.close()

# Export snapshots of 3D views from a ResInsight instance launched in headless mode.
#
# Headless mode hides all windows and uses software OpenGL rendering, so no GPU is required.
# On Linux a display server is still needed to create the OpenGL context. Launch the bundled
# 'resinsight' script instead of the 'ResInsight' binary, as the script automatically wraps the
# application in xvfb-run when no display is available (requires the xvfb package).

import os
import sys

import rips

# The executable path is used both by rips.Instance.launch() and to locate the bundled TestModels
resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")
if not resinsight_exe_path:
    sys.exit(
        "Set the environment variable RESINSIGHT_EXECUTABLE to the path of the "
        "ResInsight executable (Linux: the bundled 'resinsight' launcher script)."
    )

# This requires the TestModels to be installed with ResInsight (RESINSIGHT_BUNDLE_TESTMODELS):
resinsight_install_path = os.path.dirname(resinsight_exe_path)
test_models_path = os.path.join(resinsight_install_path, "TestModels")
path_name = os.path.join(
    test_models_path, "TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
)
if not os.path.isfile(path_name):
    sys.exit(
        "Could not find the test model "
        + path_name
        + ". This example requires the TestModels to be installed with ResInsight."
    )

# Launch ResInsight without showing any windows. Raises rips.RipsError on failure.
resinsight = rips.Instance.launch(command_line_parameters=["--headless"])

# Always shut down the headless instance, as it has no window the user could close
try:
    case = resinsight.project.load_case(path_name)

    # Create a view, as the case is loaded without any views
    view = case.create_view()
    view.apply_cell_result(
        result_type=rips.PropertyType.DYNAMIC_NATIVE, result_variable="SOIL"
    )

    snapshot_folder = os.path.join(os.getcwd(), "snapshots")
    resinsight.set_export_folder(
        export_type="SNAPSHOTS", path=snapshot_folder, create_folder=True
    )

    # Export a snapshot of all 3D views in the project
    resinsight.project.export_snapshots(
        content_type=rips.SnapshotContentType.VIEWS, width=800, height=600
    )

    exported_files = (
        sorted(os.listdir(snapshot_folder)) if os.path.isdir(snapshot_folder) else []
    )
    for file_name in exported_files:
        print("Exported snapshot: " + os.path.join(snapshot_folder, file_name))
finally:
    resinsight.exit()

if not exported_files:
    sys.exit("No snapshots were exported to " + snapshot_folder)

#############################################################
# This example launches ResInsight with a GUI, loads a model
# and rotates the camera around the model. ResInsight is left
# open afterwards, unless frames are exported.
#
# Optionally pass a folder as the first argument to export
# each frame as an image, for example to create a movie:
#   python view_rotate_animation.py frames
#
# Requires the environment variable RESINSIGHT_EXECUTABLE.
# The TestModels must be installed with ResInsight
# (RESINSIGHT_BUNDLE_TESTMODELS), or set the environment variable
# MODEL_PATH to an .EGRID file.
#############################################################
import os
import sys
import time

import rips

TILT = 60  # Degrees from straight down
STEP = 5  # Degrees between frames
FRAME_DELAY = 0.05  # Seconds to wait between frames
TURNS = 3  # Number of full turns when not exporting frames

resinsight_exe_path = os.environ.get("RESINSIGHT_EXECUTABLE")
if not resinsight_exe_path:
    sys.exit("Set the environment variable RESINSIGHT_EXECUTABLE")

MODEL_PATH = os.environ.get("MODEL_PATH") or os.path.join(
    os.path.dirname(resinsight_exe_path),
    "TestModels",
    "TEST10K_FLT_LGR_NNC",
    "TEST10K_FLT_LGR_NNC.EGRID",
)
if not os.path.isfile(MODEL_PATH):
    sys.exit("Could not find the model " + MODEL_PATH)

export_folder = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else None

resinsight = rips.Instance.launch()

try:
    case = resinsight.project.load_case(MODEL_PATH)
    views = case.views()
    view = views[0] if views else case.create_view()
    view.apply_cell_result(
        result_type=rips.PropertyType.DYNAMIC_NATIVE, result_variable="SOIL"
    )

    if export_folder:
        os.makedirs(export_folder, exist_ok=True)

    # Azimuth is the compass bearing of the viewing direction. The camera orbits the center of the
    # view and keeps the distance to it, so fit the whole model first.
    turns = 1 if export_folder else TURNS
    view.zoom_all()
    frames = list(range(0, 360, STEP)) * turns
    for frame, azimuth in enumerate(frames):
        view.set_camera(azimuth=azimuth, tilt=TILT)
        if export_folder:
            view.export_snapshot(
                export_folder=export_folder, prefix=f"frame_{frame:04d}_"
            )
        else:
            time.sleep(FRAME_DELAY)
finally:
    # Leave ResInsight open when watching the animation, but close it after exporting frames
    if export_folder:
        resinsight.exit()

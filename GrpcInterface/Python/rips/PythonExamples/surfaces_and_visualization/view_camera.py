#############################################################
# This example controls the camera of a 3D view.
# The camera always looks at the center of the model.
# Requires a ResInsight instance with a GUI, as the camera
# belongs to the 3D viewer.
#############################################################
import sys

import rips

resinsight = rips.Instance.find()

cases = resinsight.project.cases()
if not cases:
    sys.exit("No cases found in the project")

views = cases[0].views()
if not views:
    sys.exit("The first case has no views")
view = views[0]

# The camera orbits the center of the view, keeping the distance to it. Start by fitting
# the whole model in the view to get a known distance.
view.zoom_all()

# Look at the model from the south-southwest, tilted 60 degrees from vertical.
# Azimuth is the compass bearing of the viewing direction (0 = north, 90 = east) and
# tilt is the angle from straight down (0 = looking down, 90 = horizontal).
view.set_camera(azimuth=200, tilt=60)

# Move closer to the center. zoom is relative to the current distance, so 2 halves it.
view.set_camera(azimuth=200, tilt=60, zoom=2.0)

# Standard directions, as in the View menu
view.set_view_from(rips.ViewFrom.NORTH)
view.set_view_from(rips.ViewFrom.TOP)

# Fit the whole model in the view, keeping the viewing direction
view.zoom_all()

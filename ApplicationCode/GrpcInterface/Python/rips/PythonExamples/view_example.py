#############################################################
# This example will alter the views of all cases
# By setting the background color and toggle the grid box
# Also clones the first view
#############################################################
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Check if connection worked
if resinsight is not None:
    # Get a list of all cases
    cases = resinsight.project.cases()
    for case in cases:
        # Get a list of all views
        views = case.views()
        for view in views:
            # Set some parameters for the view
            view.show_grid_box = not view.show_grid_box
            view.background_color = "#3388AA"
            # Update the view in ResInsight
            view.update()
        # Clone the first view
        new_view = views[0].clone()
        new_view.background_color = "#FFAA33"
        new_view.update()
        view.show_grid_box = False
        view.set_visible(False)
        view.update()

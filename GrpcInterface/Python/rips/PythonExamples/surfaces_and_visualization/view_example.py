#############################################################
# This example will alter the views of all cases
# By setting the background color and toggle the grid box
# Also clones the first view
#############################################################
import rips

# Connect to ResInsight instance
resinsight = rips.Instance.find()

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

# Tile all visible 3D views
resinsight.project.tile_views()

# Link the views, then unlink the last view from the group
project_views = resinsight.project.views()
if len(project_views) >= 2:
    resinsight.project.link_views(views=project_views)
    resinsight.project.unlink_views(views=[project_views[-1]])

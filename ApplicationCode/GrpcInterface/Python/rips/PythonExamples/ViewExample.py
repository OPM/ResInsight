#############################################################
# This example will alter the views of all cases
# By setting the background color and toggle the grid box
# Also clones the first view
#############################################################
import rips
# Connect to ResInsight instance
resInsight = rips.Instance.find()

# Check if connection worked
if resInsight is not None:
    # Get a list of all cases
    cases = resInsight.project.cases()
    for case in cases:
        # Get a list of all views
        views = case.views()
        for view in views:
            # Set some parameters for the view
            view.setShowGridBox(not view.showGridBox())
            view.setBackgroundColor("#3388AA")            
            # Update the view in ResInsight
            view.update()
        # Clone the first view
        newView = views[0].clone()
        view.setShowGridBox(False)
        newView.setBackgroundColor("#FFAA33")
        newView.update()
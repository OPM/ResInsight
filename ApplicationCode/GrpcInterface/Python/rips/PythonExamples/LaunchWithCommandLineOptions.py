# Load ResInsight Processing Server Client Library
import rips
# Launch ResInsight with last project file and a Window size of 600x1000 pixels
resInsight = rips.Instance.launch(commandLineParameters=['--last', '--size', 600, 1000])
# Get a list of all cases
cases = resInsight.project.cases()

print ("Got " + str(len(cases)) + " cases: ")
for case in cases:
    print("Case name: " + case.name)
    print("Case grid path: " + case.grid_path())

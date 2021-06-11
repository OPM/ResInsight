# Load ResInsight Processing Server Client Library
import rips

# Launch ResInsight with last project file and a Window size of 600x1000 pixels
resinsight = rips.Instance.launch(
    command_line_parameters=["--last", "--size", 600, 1000]
)
# Get a list of all cases
cases = resinsight.project.cases()

print("Got " + str(len(cases)) + " cases: ")
for case in cases:
    print("Case name: " + case.name)
    print("Case grid path: " + case.file_path)

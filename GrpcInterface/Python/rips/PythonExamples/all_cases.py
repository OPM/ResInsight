###################################################################################
# This example will connect to ResInsight, retrieve a list of cases and print info
#
###################################################################################

# Import the ResInsight Processing Server Module
import rips

# Connect to ResInsight
resinsight = rips.Instance.find()
if resinsight is not None:
    # Get a list of all cases
    cases = resinsight.project.cases()

    print("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print("Case id: " + str(case.id))
        print("Case name: " + case.name)
        print("Case type: " + case.__class__.__name__)
        print("Case file name: " + case.file_path)
        print("Case reservoir bounding box:", case.reservoir_boundingbox())

        timesteps = case.time_steps()
        for t in timesteps:
            print("Year: " + str(t.year))
            print("Month: " + str(t.month))

        if isinstance(case, rips.EclipseCase):
            print("Getting coarsening info for case: ", case.name, case.id)
            coarsening_info = case.coarsening_info()
            if coarsening_info:
                print("Coarsening information:")

            for c in coarsening_info:
                print(
                    "[{}, {}, {}] - [{}, {}, {}]".format(
                        c.min.x, c.min.y, c.min.z, c.max.x, c.max.y, c.max.z
                    )
                )

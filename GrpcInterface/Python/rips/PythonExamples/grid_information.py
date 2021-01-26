######################################################################################
# This example prints information about the grids of all cases in the current project
######################################################################################

import rips

resinsight = rips.Instance.find()

cases = resinsight.project.cases()
print("Number of cases found: ", len(cases))
for case in cases:
    print(case.name)
    grids = case.grids()
    print("Number of grids: ", len(grids))
    for grid in grids:
        print("Grid dimensions: ", grid.dimensions())

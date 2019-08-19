import rips

resInsight  = rips.Instance.find()
cases = resInsight.project.cases()
print("Number of cases found: ", len(cases))
for case in cases:
    print(case.name)
    grids = case.grids()
    print("Number of grids: ", len(grids))
    for grid in grids:
        print("Grid dimensions: ", grid.dimensions())




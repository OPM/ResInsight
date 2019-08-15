import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    cases = resInsight.project.selectedCases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        for property in case.properties.available('DYNAMIC_NATIVE'):
            print(property)



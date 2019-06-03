import sys
import os
import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    cases = resInsight.project.cases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)

import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    cases = resInsight.project.cases()

    print ("Got " + str(len(cases)) + " cases: ")
    for case in cases:
        print(case.name)
        views = case.views()
        for view in views:
            view.setShowGridBox(not view.showGridBox())
            view.setBackgroundColor("#3388AA")
            view.update()

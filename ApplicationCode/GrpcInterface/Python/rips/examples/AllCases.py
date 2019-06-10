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
        assert(case.address() is not 0)
        assert(case.classKeyword() == "EclipseCase")
        print("\n#### Case ####")
        for keyword in case.keywords():
            print (keyword + ": " + case.getValue(keyword))
        print ("\n####Project#####")
        pdmProject = case.ancestor(classKeyword="ResInsightProject")
        assert(pdmProject)
        assert(pdmProject.address() is not 0)
        assert(pdmProject.address() == resInsight.project.address())

        for keyword in resInsight.project.keywords():
            print (keyword + ": " + resInsight.project.getValue(keyword))
        pdmViews = resInsight.project.descendants(classKeyword="ReservoirView")
        for view in pdmViews:
            print ("\n####View####")
            print(view.classKeyword(), view.address())
            for viewKeyword in view.keywords():
                print(viewKeyword + "-> " + str(view.getValue(viewKeyword)))
            view.setValue("ShowGridBox", not view.getValue("ShowGridBox"))
            view.setValue("ViewBackgroundColor", "#3388AA")
            view.update()
        
            print ("\n####Cell Result####")
            cellResults = view.children(classKeyword="ResultSlot")
            for resultKeyword in cellResults[0].keywords():
                print(resultKeyword + "->" + str(cellResults[0].getValue(resultKeyword)))
            cellResults[0].setValue("ResultVariable", "SOIL")
            cellResults[0].setValue("ResultType", "DYNAMIC_NATIVE")
            cellResults[0].update()
import rips

resInsight  = rips.Instance.find()

view = resInsight.project.view(0)

cellResult = view.cellResult()

cellResult.printObjectInfo()

cellResult.setValue("ResultType", "FLOW_DIAGNOSTICS")
cellResult.setValue("ResultVariable", "TOF")

cellResult.update()
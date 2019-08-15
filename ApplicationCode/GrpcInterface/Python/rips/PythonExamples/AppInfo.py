import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    print(resInsight.app.versionString())
    print("Is this a console run?", resInsight.app.isConsole())

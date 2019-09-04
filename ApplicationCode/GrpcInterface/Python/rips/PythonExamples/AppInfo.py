import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    print(resInsight.versionString())
    print("Is this a console run?", resInsight.isConsole())

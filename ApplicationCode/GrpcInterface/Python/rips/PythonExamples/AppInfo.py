import rips

resinsight  = rips.Instance.find()
if resinsight is not None:
    print(resinsight.versionString())
    print("Is this a console run?", resinsight.isConsole())

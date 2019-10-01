import rips

resinsight  = rips.Instance.find()
if resinsight is not None:
    print(resinsight.version_string())
    print("Is this a console run?", resinsight.is_console())

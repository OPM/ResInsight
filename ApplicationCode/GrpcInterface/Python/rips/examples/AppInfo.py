import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

resInsight  = rips.Instance.find()
if resInsight is not None:
    print(resInsight.app.versionString())
    print("Is this a console run?", resInsight.app.isConsole())

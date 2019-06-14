import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

resInsight  = rips.Instance.find()

if resInsight is None:
    print('ERROR: could not find ResInsight')
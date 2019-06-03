import sys
import os
import tempfile
sys.path.insert(1, os.path.join(sys.path[0], '..'))
import rips

# Load instance
resInsight = rips.Instance.find()

# Run a couple of commands
resInsight.commands.setTimeStep(caseId=0, timeStep=3)
resInsight.commands.setMainWindowSize(width=800, height=500)
#resInsight.commands.exportWellPaths()
with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
    print("Temporary folder: ", tmpdirname)
    resInsight.commands.setExportFolder(type='SNAPSHOTS', path=tmpdirname)
    resInsight.commands.exportSnapshots()
    print(os.listdir(tmpdirname))
    assert(len(os.listdir(tmpdirname)) > 0)
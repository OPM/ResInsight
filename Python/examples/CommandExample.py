import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '../api'))
import ResInsight

# Load instance
resInsight = ResInsight.Instance.find()

# Run a couple of commands
resInsight.commands.setTimeStep(caseId=0, timeStep=3)
resInsight.commands.setMainWindowSize(width=800, height=500)
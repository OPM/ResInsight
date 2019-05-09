import ResInsight

# Load instance
resInsight = ResInsight.Instance()

# Run a couple of commands
resInsight.commands.setTimeStep(caseId=0, timeStep=3)
resInsight.commands.setMainWindowSize(width=800, height=500)
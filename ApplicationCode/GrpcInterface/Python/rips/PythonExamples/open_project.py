# Load ResInsight Processing Server Client Library
import rips
# Connect to ResInsight instance
resinsight = rips.Instance.find()

# Open a project
resinsight.project.open(
    "C:/Users/lindk/Projects/ResInsight/TestModels/TEST10K_FLT_LGR_NNC/10KWithWellLog.rsp")

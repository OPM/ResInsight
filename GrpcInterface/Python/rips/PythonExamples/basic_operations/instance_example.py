#######################################
# This example connects to ResInsight
#######################################
import rips

try:
    resinsight = rips.Instance.find()
    print("Successfully connected to ResInsight")
except rips.RipsError as e:
    print(f"ERROR: could not find ResInsight: {e}")

#######################################
# This example connects to ResInsight
#######################################
import rips

resinsight  = rips.Instance.find()

if resinsight is None:
    print('ERROR: could not find ResInsight')
else:
	print('Successfully connected to ResInsight')
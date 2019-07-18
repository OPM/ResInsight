import sys
import os

sys.path.insert(1, os.path.join(sys.path[0], '../../'))
import rips

resInsight  = rips.Instance.find()

casePaths = []
casePaths.append("../../../../TestModels/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID")
casePaths.append("../../../../TestModels/Case_with_10_timesteps/Real10/BRUGGE_0010.EGRID")
groupId, groupName = resInsight.commands.createGridCaseGroup(casePaths=casePaths)
print("Group id = " + str(groupId))
print("Group name = " + groupName)
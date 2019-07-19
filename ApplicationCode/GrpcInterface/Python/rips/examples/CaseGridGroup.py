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

caseId = resInsight.commands.createStatisticsCase(caseGroupId=groupId)

caseGroups = resInsight.project.descendants("RimIdenticalGridCaseGroup");

caseIds = []
for caseGroup in caseGroups:
    print ("#### Case Group ####")
    for kw in caseGroup.keywords():
        print (kw, caseGroup.getValue(kw))

    for caseCollection in caseGroup.children("StatisticsCaseCollection"):
        print ("  ### Case Collection ###")
        statCases = caseCollection.children("Reservoirs")                    

        for statCase in statCases:
            print("   ## Stat Case ##")
            for skw in statCase.keywords():
                print("   ", skw, statCase.getValue(skw))
            statCase.setValue("DynamicPropertiesToCalculate", statCase.getValue("DynamicPropertiesToCalculate") + ["SWAT"])
            statCase.update()
            caseIds.append(statCase.getValue("CaseId"))

print(caseIds)
resInsight.commands.computeCaseGroupStatistics(caseIds=caseIds)
        
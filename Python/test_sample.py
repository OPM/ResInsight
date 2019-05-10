

import ResInsight


# content of test_sample.py
def getActiveCellCount(caseId):
  resInsight = ResInsight.Instance()
  activeCellInfo = resInsight.gridInfo.getAllActiveCellInfos(caseId)

  receivedActiveCells = []
  for activeCell in activeCellInfo:
  	receivedActiveCells.append(activeCell)
  
  return len(receivedActiveCells)

def myOpenProject(filepath):
  resInsight = ResInsight.Instance()
  #resInsight.commands.setMainWindowSize(width=800, height=500)
  resInsight.commands.openProject(filepath)





def test_openProjectAndCountCells():
  testRepositoryRoot = "d:\\gitroot-ceesol\\ResInsight-regression-test"

  #casePath = testRepositoryRoot + "\\ModelData\\TEST10K_FLT_LGR_NNC\\TEST10K_FLT_LGR_NNC.EGRID"
  #openEclipseCase(casePath)

#  projectPath = testRepositoryRoot + "\\ProjectFiles\\ProjectFilesSmallTests\\TestCase_10K_Complete\\RegressionTest.rsp" 
#  projectPath = testRepositoryRoot + "\\ProjectFiles\\ProjectFilesSmallTests\\TestCase_Norne\\RegressionTest.rsp" 
  projectPath = testRepositoryRoot + "\\ProjectFiles\\ProjectFilesSmallTests\\TestCase_10K_Watertight\\RegressionTest.rsp"
  myOpenProject(projectPath)

  assert getActiveCellCount(0) == 11125
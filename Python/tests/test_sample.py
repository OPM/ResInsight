
import os, sys

# Add the 'api' path to system path to be able to import modules from the 'api' folder
# python current working directory must be 'tests' 
sys.path.insert(1, os.path.join(sys.path[0], '..\\api'))

import ResInsight

resInsight = ResInsight.Instance()


# content of test_sample.py
def getActiveCellCount(caseId):
  activeCellInfoChunks = resInsight.gridInfo.streamActiveCellInfo(caseId)

  receivedActiveCells = []
  for activeCellChunk in activeCellInfoChunks:
	  for activeCell in activeCellChunk.data:
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



def test_openCaseAndCountCells():
  testRepositoryRoot = "d:\\gitroot-ceesol\\ResInsight-regression-test"

  casePath = testRepositoryRoot + "\\ModelData\\TEST10K_FLT_LGR_NNC\\TEST10K_FLT_LGR_NNC.EGRID"
  resInsight.commands.loadCase(casePath)

  assert getActiveCellCount(0) == 11125

  resInsight.commands.closeProject()

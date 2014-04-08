---
layout: docs
prev_section: regressiontestsystem
next_section: home
title: Command Line Parameters
permalink: /docs/commandlineparameters/
---

## Command Line Parameters #

| Parameter | Description |
|-----------|-------------|
| `--last`                   | Open last used project. |
| `--project _filename_`     | Open project file _filename_. |
| `--case _casename_`        | Import Eclipse case _casename_ (do not include .GRID/.EGRID) |
| `--startdir _folder_`      | Set startup directory. |
| `--savesnapshots`          | Save snapshot of all views to 'snapshots' folder in project file folder. Application closes after snapshots have been written. |
| `--size _width_ _height_`  | Set size of the main application window. |
| `--replaceCase [_caseId_] _newGridFile_`  | Replace grid in _caseId_ or first case with _newgridFile_. |
| `--replaceSourceCases [_caseGroupId_] _gridListFile_` | Replace source cases in _caseGroupId_ or first grid case group with the grid files listed in the _gridListFile_ file. |
| `--multiCaseSnapshots _gridListFile_` | For each grid file listed in the *gridListFile* file, replace the first case in the project and save snapshot of all views. |
| `--help, -?`               | Displays help text and version info |
| `--regressiontest _folder_` | Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder. |
| `--updateregressiontestbase _folder_` | For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely. |

See also the [Regression Test System ](RegressionTestSystem.md) for a more in-depth explanation.


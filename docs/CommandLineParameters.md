[ Contents ](UsersGuide.md#contents)

------
## Command Line Parameters #

| Parameter | Description |
|-----------|-------------|
| `--last`                   | Open last used project. |
| `--project <filename>`     | Open project file <filename>. |
| `--case <casename>`        | Import Eclipse case <casename> (do not include .GRID/.EGRID) |
| `--startdir <folder>`      | Set startup directory. |
| `--savesnapshots`          | Save snapshot of all views to 'snapshots' folder in project file folder. Application closes after snapshots have been written. |
| `--size <width> <height>`  | Set size of the main application window. |
| `--replaceCase [<caseId>] <newGridFile>`  | Replace grid in <caseId> or first case with <newgridFile>. |
| `--replaceSourceCases [<caseGroupId>] <gridListFile>` | Replace source cases in <caseGroupId> or first grid case group with the grid files listed in the <gridListFile> file. |
| `--multiCaseSnapshots <gridListFile>` | For each grid file listed in the <gridListFile> file, replace the first case in the project and save snapshot of all views. |
| `--help, -?`               | Displays help text and version info |
| `--regressiontest <folder>` | Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder. |
| `--updateregressiontestbase <folder>` | For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely. |

See also the [Regression Test System ](RegressionTestSystem.md) for a more in-depth explanation.

------
[ Contents ](UsersGuide.md#contents)

## Command Line Parameters #

Parameter|Description
---------|-----------
`-help, -?`               | Displays help text and version info
`-last`                   | Open last used project
`-project <filename>`     | Open project file <filename>
`-case <casename>`        | Import Eclipse case <casename> (do not include .GRID/.EGRID)
`-savesnapshots`          | Save snapshot of all views to 'snapshots' folder in project file folder. Application closes after snapshots are written to file.
`-regressiontest <folder>` | Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder.
`-updateregressiontestbase <folder>` | For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely.

See also the [Regression Test System ](RegressionTestSystem.md) for a more in-depth explanation.
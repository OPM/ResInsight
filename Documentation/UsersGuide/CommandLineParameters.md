# Command Line Parameters #

## Basic usage ##

Parameter|Description
---------|-----------
`-last`                   | Open last used project
`-project <filename>`     | Open project file <filename>
`-case <casename>`        | Import Eclipse case <casename> (do not include .GRID/.EGRID)
`-startdir`               | The default directory for open/save commands
`-help, -?`               | Displays help text

## Regression test ##
A regression tool for QA is build into ResInsight. This tool will read a project file, open all views in this project, save snapshot images to file, and close the project. When snapshot images from all projects are completed, difference images based on generated and QA-approved images are computed. Based on these three sets of images, an HTML report is created and automatically displayed.

### Regression test folder structure ###

```
MyRegressionTestFolder
  TestCase_Faults
    RegTestBaseImages
    RegTestDiffImages
    RegTestGeneratedImages
  TestCase_SOIL
    RegTestBaseImages
    RegTestDiffImages
    RegTestGeneratedImages
  ...
```

`-savesnapshots`
Save snapshot of all views to 'snapshots' folder in project file folder. Application closes after snapshots are written to file.

`-regressiontest <folder>`
Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based 
on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder.

`-updateregressiontestbase <folder>`
For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely.

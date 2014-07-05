---
layout: docs
prev_section: regressiontestsystem
next_section: home
title: Command Line Parameters
permalink: /docs/commandlineparameters/
published: true
---

## Command Line Parameters #

| Parameter | Description |
|-----------|-------------|
| --last                   | Open last used project. |
| --project &lt;filename&gt;     | Open project file &lt;filename&gt;. |
| --case &lt;casename&gt;        | Import Eclipse case &lt;casename&gt; (do not include .GRID/.EGRID) |
| --startdir &lt;folder&gt;      | Set startup directory. |
| --savesnapshots          | Save snapshot of all views to 'snapshots' folder in project file folder. Application closes after snapshots have been written. |
| --size &lt;width&gt; &lt;height&gt;  | Set size of the main application window. |
| --replaceCase [&lt;caseId&gt;] &lt;newGridFile&gt;  | Replace grid in &lt;caseId&gt; or first case with &lt;newgridFile&gt;. |
| --replaceSourceCases [&lt;caseGroupId&gt;] &lt;gridListFile&gt; | Replace source cases in &lt;caseGroupId&gt; or first grid case group with the grid files listed in the &lt;gridListFile&gt; file. |
| --multiCaseSnapshots &lt;gridListFile&gt; | For each grid file listed in the &lt;gridListFile&gt; file, replace the first case in the project and save snapshot of all views. |
| --help, -?               | Displays help text and version info |
| --regressiontest &lt;folder&gt; | Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder. |
| --updateregressiontestbase &lt;folder&gt; | For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely. |

See also the [Regression Test System ](RegressionTestSystem.md) for a more in-depth explanation.
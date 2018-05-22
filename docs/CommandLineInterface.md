---
layout: docs
title: Command Line Interface
permalink: /docs/commandlineinterface/
published: true
---

ResInsight supports several command line parameters that can be used to automate some tasks using shell scripts. 

Command line parameters are prefixed using a double dash. This convention is used on all platforms to make it possible to reuse scripts across different platforms. See GNU Standards for [Command Line Interfaces](http://www.gnu.org/prep/standards/html_node/Command_002dLine-Interfaces.html#Command_002dLine-Interfaces).

Examples on how command line options are used are given [below]({{ site.baseurl }}/docs/commandlineinterface#usage-examples)

| Parameter | Description |
|-----------|-------------|
| &#45;&#45;last                   | Open last used project. |
| &#45;&#45;project &lt;filename&gt;     | Open project file &lt;filename&gt;. |
| &#45;&#45;case &lt;casename&gt;        | Import Eclipse case &lt;casename&gt; (do not include .GRID/.EGRID) |
| &#45;&#45;startdir &lt;folder&gt;      | Set startup directory. |
| &#45;&#45;savesnapshots all&#124;views&#124;plots&#124;          | Save snapshot of all views/plots or all(both views and plots) to **snapshots** folder in project file folder. If no argument option is given, views are exported. Application closes after snapshots have been written. |
| &#45;&#45;size &lt;width&gt; &lt;height&gt;  | Set size of the main application window. |
| &#45;&#45;replaceCase [&lt;caseId&gt;] &lt;newGridFile&gt;  | Replace grid in &lt;caseId&gt; or first case with &lt;newGridFile&gt;. Repeat parameter for multiple replace operations.|
| &#45;&#45;replaceSourceCases [&lt;caseGroupId&gt;] &lt;gridListFile&gt; | Replace source cases in &lt;caseGroupId&gt; or first grid case group with the grid files listed in the &lt;gridListFile&gt; file. Repeat parameter for multiple replace operations.|
| &#45;&#45;multiCaseSnapshots &lt;gridListFile&gt; | For each grid file listed in the &lt;gridListFile&gt; file, replace the first case in the project and save snapshot of all views. |
| &#45;&#45;commandFile &lt;commandFile&gt; | Execute a command file. See [command file documentation.]({{site.baseurl}}/docs/commandfile) |
| &#45;&#45;commandFileProject &lt;filename&gt; | Project to use if performing case looping for command file. Used in conjunction with `commandFileReplaceCases`. |
| &#45;&#45;commandFileReplaceCases [&lt;caseId&gt;] &lt;caseListFile&gt; | Supply list of cases to replace in project, performing command file for each case. Project to replace cases in must be set with `commandFileProject`. If caseId is not supplied, first case is replaced. When supplying caseId, multiple cases may be replaced at once, by supplying several caseIds and a file containing a list of grid-files to replace with for each caseId. |
| &#45;&#45;help, &#45;&#45;?       | Displays help text and version info |
| &#45;&#45;regressiontest &lt;folder&gt; | Run a regression test on all sub-folders starting with `TestCase*` of the given folder. **RegressionTest.rip** files in the sub-folders will be opened and snapshots of all the views is written to the sub-sub-folder **RegTestGeneratedImages**. Then difference images is generated in the sub-sub-folder **RegTestDiffImages** based on the images in sub-sub-folder **RegTestBaseImages**. The results are presented in **ResInsightRegressionTestReport.html** that is written in the given folder. |
| &#45;&#45;updateregressiontestbase &lt;folder&gt; | For all sub-folders starting with `TestCase*`, copy the images in the sub-sub-folder **RegTestGeneratedImages** to the sub-sub-folder **RegTestBaseImages** after deleting **RegTestBaseImages** completely. |
| &#45;&#45;unittest | Execute integration tests |

See also the [Regression Test System ]({{site.baseurl }}/docs/regressiontestsystem) for a more in-depth explanation.

<div class="note">
<h5>Reduce project load time using <code>--replaceSourceCases</code> </h5>
  
ResInsight stores data computed by statistics calculation in a cache file. When a project file is loaded, data from this cache is also imported. For large cases, the cached data can be large. When replacing source cases during batch, this data is never used and can be removed from the cache using the following workaround:
<ul>
  <li>Open the project file used to produce statistics</li>
  <li>Select the statistics object in the project tree</li>
  <li>Click the button <b>Edit (Will DELETE current result)</b></li>
  <li>Save the project file</li>
</ul> 
</div>

## Usage Examples 

These examples are also available from the [test section](https://github.com/OPM/ResInsight/tree/master/TestModels/Case_with_10_timesteps).

### Example 1 : Create snapshots of all views for multiple cases
A list of cases is defined in **CaseList.txt**, containing the following

```
Real0/BRUGGE_0000.EGRID
Real10/BRUGGE_0010.EGRID
Real30/BRUGGE_0030.EGRID
Real40/BRUGGE_0040.EGRID
```

The command line used to run this example is shown here:

```
ResInsight --project BatchTest.rsp --multiCaseSnapshots CaseList.txt --size 500 500
```

This will instruct ResInsight to read the project file **BatchTest.rsp**. All cases will be replaced one by one in ResInsight, and snapshots of all views will be written to file. 


### Example 2 : Replace a single case and take snapshots of all views

The command line used to run this example is shown here:

```
ResInsight --project BatchTest.rsp --replaceCase "Real10\BRUGGE_0010.EGRID" --savesnapshots
```

This will instruct ResInsight to read the project file **BatchTest.rsp**. The specified case **Real10\BRUGGE_0010.EGRID** will be imported into the project, and snapshots of all views will be written to file. 


### Example 3 : Replace source cases in a case group and create snapshot
A list of cases is defined in **CaseList2.txt**, containing the following

```
Real0/BRUGGE_0000.EGRID
Real10/BRUGGE_0010.EGRID
```

The command line used to run this example is shown here:

```
ResInsight --project BatchStatistics.rsp --replaceSourceCases CaseList2.txt --savesnapshots
```

This will instruct ResInsight to read the project file **BatchStatistics.rsp**. All cases specified will be imported in the case group specified in the project file. Statistics will be computed, and snapshots for all views will be written to file.

### Example 4 : Replace source cases in multiple case groups and create snapshots
Multiple source case groups can be updated by repeating the replaceSourceCases parameter.

The command line used to run this example is shown here:

```
ResInsight --project BatchStatistics.rsp --replaceSourceCases 0 CaseList2.txt --replaceSourceCases 1 CaseList3.txt --savesnapshots
```
This will instruct ResInsight to read the project file **BatchStatistics.rsp**. Source cases for case group 0 is given in CaseList2.txt, and source cases for case group 1 is given in CaseList3.txt. Statistics will be computed, and snapshots for all views will be written to file.

The possibility to replace multiple cases can also be applied for single case replace (parameter *replaceCase*).

---
layout: docs
prev_section: faults
next_section: regressiontestsystem
title: Batch Commands
permalink: /docs/batchcommands/
published: true
---

ResInsight supports several commands useful in a batch setting. These examples are available from the [test section](https://github.com/OPM/ResInsight/tree/master/TestModels/Case_with_10_timesteps).

See also [Command Line Arguments]({{ site.baseurl }}/docs/commandlineparameters) for an overview of all command line arguments.

## Example 1 : Create snapshots of all views for multiple cases
A list of cases is defined in **CaseList.txt**, containing the following

    Real0/BRUGGE_0000.EGRID
    Real10/BRUGGE_0010.EGRID
    Real30/BRUGGE_0030.EGRID
    Real40/BRUGGE_0040.EGRID

The command line used to run this example is shown here:

    ResInsight --project BatchTest.rsp --multiCaseSnapshots CaseList.txt --size 500 500

This will instruct ResInsight to read the project file **BatchTest.rsp**. All cases will be replaced one by one in ResInsight, and snapshots of all views will be written to file. 


## Example 2 : Replace a single case and take snapshots of all views

The command line used to run this example is shown here:

    ResInsight --project BatchTest.rsp --replaceCase "Real10\BRUGGE_0010.EGRID" --savesnapshots

This will instruct ResInsight to read the project file **BatchTest.rsp**. The specified case **Real10\BRUGGE_0010.EGRID** will be imported into the project, and snapshots of all views will be written to file. 


## Example 3 : Replace source cases in a case group and create snapshot
A list of cases is defined in **CaseList2.txt**, containing the following

    Real0/BRUGGE_0000.EGRID
    Real10/BRUGGE_0010.EGRID

The command line used to run this example is shown here:

    ResInsight --project BatchStatistics.rsp --replaceSourceCases CaseList2.txt --savesnapshots

This will instruct ResInsight to read the project file **BatchTest.rsp**. All cases specified will be imported in the case group specified in the project file. Statistics will be computed, and snapshots for all views will be written to file.
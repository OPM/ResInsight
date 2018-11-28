---
layout: docs
title: Grid Import
permalink: /docs/gridimportexport/
published: true
---

## Importing Eclipse Cases 
ResInsight supports the following type of Eclipse input data:

- _`*.GRID`_ and _`*.EGRID`_ files along with their _`*.INIT`_ and restart files _`*.XNNN`_ and _`*.UNRST`_. 
- Grid and Property data from  _`*.GRDECL`_ files.

<div class="note">
Release 2018.11 supports import of simulations from Intersect. To be able to import into ResInsight, the Intersect simulation must be exported into Eclipse file format.
</div>

### Eclipse Results
ResInsight offers several ways to import Eclipse (grid) files. Use one of the following commands in the **File->Import->Eclipse Cases** menu:
- **Import Eclipse Case**: Brings up the standard file selection dialog. Select _`*.EGRID`_ or _`*.GRID`_ Eclipse files for import. Multiple selections are allowed.
- **Import Eclipse Cases Recursively**: Brings up the recursive file selection dialog. This dialog is described in detail on the [Summary Plots page]({{site.baseurl}}/docs/summaryplots/#recursive-summary-file-import).
- **Import Eclipse Case (Time Step Filtered)**: See [description](#time-step-filtered-eclipse-result)
- **Import Input Eclipse Case**: See [description](#eclipse-ascii-input-data)
- **Create Grid Case Group from Files** and **Create Grid Case Group from Files Recursively**: These commands import a number of Eclipse files and places the cases in a [grid case group]({{site.baseurl}}/docs/casegroupsandstatistics/#creating-grid-case-groups). The only difference between the two commands, is the dialog used to select files. The recursive version is using the [recursive file selection dialog]({{site.baseurl}}/docs/summaryplots/#recursive-summary-file-import) and is considered the new way of selecting files.

The **Reload Case** command can be used to reload a previously imported case, to make sure it is up to date. This is useful if the grid or result files changes while a ResInsight session is active.

<div class="note">
You can select several grid files in one go by multiple selection of files (Ctrl + left mouse button, Shift + left mouse button). 
</div>

#### Result Index File

If enabled, ResInsight will generate an index file when reading the eclipse result files for the first time. This file will significantly reduce the time used to open the case next time. The file is named _`<casename>.RESINSIGHT_IDX`_
See [Preferences: Behavior When Loading Data]({{ site.baseurl }}/docs/preferences#behavior-when-loading-data)

### Time Step Filtered Eclipse Result
Some Eclipse files have an enormous amount of time steps. If only a selection of the time steps really are needed for the session, the time steps can be filtered before loading. This can possibly speed up the import a great deal. Filtering can be done in the following way.

Select **File->Import->Eclipse Cases-> ![]({{ site.baseurl }}/images/Case24x24.png) Import Eclipse Case (Time Step Filtered)** and select an _`*.EGRID`_ or _`*.GRID`_ Eclipse file for import. A dialog will appear.

![]({{ site.baseurl }}/images/timeStepFilter.png)

Filtering can be done by adjusting the following parameters:
* First and last time step
* Step filter type and with step interval size 

First and last time step to include in the import can be chosen in their respective drop down list. All time steps found in the file are included in both lists.

**Filter Type** is set to *All* by default. This means that all time steps between the first and last chosen time step will be imported. The alternative to *All* is to skip time steps in a number of *Days*, *Weeks*, *Months*, *Quarters* or *Years*. The skipping interval is set in the text field below. After editing the **Interval** field, press *tab* to update the **Filtered Time Steps** preview, or click anywhere in the dialog. Click *Ok* to import when the filter is ready.

Filtering can also be done after import, in a case's **Property Window**.

![]({{ site.baseurl }}/images/timeStepFilterPropEditor.png)

After clicking *Reload Case*, the time steps in the toolbar will be updated.

### Eclipse ASCII Input Data
1. Select **File->Import->Eclipse Cases-> ![]({{ site.baseurl }}/images/EclipseInput24x24.png) Import Input Eclipse Case** and select a _`*.GRDECL`_ file.
2. The case is imported, and a view of the case is created
3. Right-click the **Input Properties** in the generated **Input Case** and use the context menu to import additional Eclipse Property data files.

### Handling Missing or Wrong MAPAXES

The X and Y grid data can be negated in order to make the Grid model appear correctly in ResInsight. This functionality is accessible in the **Property Editor** for all Eclipse Case types as the toggle buttons **Flip X Axis** and **Flip Y Axis** as shown in the example below.
 
![]({{ site.baseurl }}/images/CaseProperties.png)

### SourSimRL Import

Results From the simulation software SourSimRL can be imported using the **SourSim File Name** field. Importing such a file will enable result type called **SourSimRL** as explained in [Eclipse Result Types]({{ site.baseurl }}/docs/cellresults#eclipse-result-types)   

## Importing ABAQUS Odb Cases
When ResInsight is compiled with ABAQUS-odb support, _`*.odb`_ files can be imported by selecting the command:

**File->Import->Geo Mechanical Cases-> ![]({{ site.baseurl }}/images/GeoMechCase24x24.png) Import Geo Mechanical Model** 

See [Build Instructions]({{ site.baseurl }}/docs/buildinstructions) on how to compile ResInsight with odb-support.

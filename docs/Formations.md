---
layout: docs
title: Zonations/Formations
permalink: /docs/formations/
published: true
---

![]({{ site.baseurl }}/images/formations_legend.PNG)

This section will describe how to use formations for different k-layers of a case, and how to use well picks/zonations for ranges of measured depths of a well path.

## Formations for k-layers

Formation information can be utilized in ResInsight as cell colors, used in property filters and are displayed in the **Result info** panel when selecting single cells.

To use this functionality you will need to :

1. Import one or more Formation Names file(s)
2. Select the correct Formation Names file in the Case of interest

### Import of Formation Names Files

Formation Names files can be imported by using the command: **File->Import->Import Formation Names**.
The user is asked to select _`*.lyr`_ files for import.

The imported Formation Names files are then listed in the **Project Tree** in a folder named **Formations**. 

Formation Names files consists of a list of formation names and their k-range. Below is an example of a Formation Names file:

```
-- Any text as comment
'MyFormationName'                 4 - 12
'MySecondFormationName'          15 - 17
'3 k-layer thick 18,19 and 20'    3
'Last Name'                      21 - 21 
```

### Select the Formation File in a Case

If only one formation file is imported, the formation will automatically be set in the active view's case. If more than one formation file is imported at once, or if a case must change formation file, the formation file for a case can be set later on. This option is available in the **Property Editor** for a case. The formation is selected in the combo box for property **Formation Names File**.

![]({{ site.baseurl }}/images/formations_property_editor.PNG)

#### Reload of formation data
If the formation file is modified outside ResInsight, the formation data can be imported again by the context menu **Formations->Reload**. This command will import formations for the selected formation files.

### Viewing the Formation Information

#### Formations in 3D View
The formations can be visualized as a result property in **Cell Results**, **Cell Edge Result**, and **Separate Fault Result**. When selected, a special legend displaying formation names is activated.

#### Property Filter Based on Formations
Formation names are available in Property Filters as Result Type **Formation Names**. This makes it easy to filter geometry based on formation specifications.

See [ Cell Filters ]({{ site.baseurl }}/docs/filters) for details.

#### Picking in 3D View
Picking on a cell being part of a formation will display the formation name in the **Result Info** windows, in addition to other pick info for the cell.

#### Annotations on Plots
Formation can be used to annotate the following plot types:
- [Well Log Plots]({{site.baseurl}}/docs/welllogsandplots)
- [RFT Plots]({{site.baseurl}}/docs/rftplot)
- [PLT Plots]({{site.baseurl}}/docs/pltplot)
- [Well Allocation Plots]({{site.baseurl}}/docs/flowdiagnosticsplots)

![]({{ site.baseurl }}/images/formOnPlot.PNG)

For RFT and PLT Plots, **Zonation/Formation Names** can be found in the plot's **Property Editor**. Tick "Show Formations" and choose the case with the desired formations. 

In Well Log Plots and Well Allocation Plots, **Zonation/Formation Names** can be found in the **Property Editor** for a **Track** or **Branch**. In addition to choosing case, the path to show formations for must also be selected, as each track can have curves with data from more than one path.

![]({{ site.baseurl }}/images/caseFormationsPropEditor.PNG)

## Well Picks

Well Picks can be set for a single well path, defined on measured depths of the well path. Unlike formations for k-layers, formations for a well path can only be used to annotate plots. A well pick can be either a *fluid* or a *formation*.

### Import of Well Pick Files

Well Pick files can be imported by using the command: **File->Import->Import Well Picks**.
The user is asked to select _`*.csv`_ files for import.

The imported Well Pick files will be added to their associated well path, if a match on well name can be found. If not, new paths will be created, and they can all be found in **Wells** in the **Project Tree**. The file path of the formations can be found in a well path's **Property Editor**.

![]({{ site.baseurl }}/images/wellPathFormationsInPropertyEditor.PNG)

A Well Pick file is a csv-file, which uses semicolon to separate entries in a table. Below is an example of such a file:

```
Well name; Column name; Unit name; Top MD; Base MD
B-3H; FLUID; GAS;2203.2;2317.4
B-3H; FLUID; OIL;2317.4;2459
B-3H; STRAT; FANGST GP. ;2203.399902;2223.350098
B-3H; STRAT;   Ile Fm. ;2203.399902;2223.350098
B-3H; STRAT;     Ile Fm. 3 ;2203.401123;2219.26001
B-3H; STRAT;     Ile Fm. 2 ;2219.26001;2222.399902
B-3H; STRAT;       Ile Fm. 2.2 ;2219.26;2219.350098
B-3H; STRAT;       Ile Fm. 2.1 ;2219.350098;2222.399902
B-3H; STRAT;     Ile Fm. 1 ;2222.399902;2223.350098
B-3H; STRAT; BAAT GP. ;2223.350098;2979.28125
B-3H; STRAT;   Ror Fm. ;2223.350098;2285.199951
B-3H; STRAT;     Ror Fm. 2 ;2223.350098;2246
B-3H; STRAT;     Ror Fm. 1 ;2246;2285.199951
B-2H; FLUID; GAS;2144.4;2338.5
B-2H; FLUID; OIL;2338.5;2440
B-2H; STRAT; FANGST GP. ;2144.199951;2158.389893
B-2H; STRAT;   Ile Fm. ;2144.199951;2158.389893
B-2H; STRAT;     Ile Fm. 2 ;2144.201416;2156.197266
B-2H; STRAT;     Ile Fm. 1 ;2156.197266;2158.38501
```

The file must have the columns "Well name", "Unit name" (i.e. formation name), "Top MD" and "Base MD" (i.e. measured depth) to be regarded as a Well Pick file. They can be listed in any order, and all other columns will be ignorded.

The three unit names *OIL*, *GAS* and *WATER* are interpreted as *fluids*. Other unit names with only capital letters are *groups*. A unit name without an index is simply a *formation*. Unit names with one number is a *formation 1*, unit names with *one* punctuation is a *formation 2*, two punctuations, *formation 3* and so on. Indentions in column name will be ignored.

### Viewing the Well Picks
See [Annotations on plots](#annotations-on-plots). Annotations are added to plots in the same way as for k-layered formations, but the source is different.

![]({{ site.baseurl }}/images/wellFormationsPropEditor.PNG)

In the **Property Editor**, choose **Well Path** as **Source**, and all well paths with formations will be shown in the drop-down list below. All disjoint well picks for the chosen well path is shown on default. To reduce the number of annotations, the **Well Pick Filter** can be used.

![]({{ site.baseurl }}/images/wellPickFilter.png)

The **Well Pick Filter** will show formations down to the specified level. If more there are more than one formation within 0.1m of an MD, only the lowermost formation is shown. Well picks interpreted as *Fluids* are only shown by ticking the **Show Fluids** mark.

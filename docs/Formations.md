---
layout: docs
title: Formations
permalink: /docs/formations/
published: true
---

![]({{ site.baseurl }}/images/formations_legend.PNG)

Formation information can be utilized in ResInsight as cell colors, used in property filters and are displayed in the **Result info** panel when selecting single cells.

To use this functionality you will need to :

1. Import one or more Formation Names file(s)
2. Select the correct Formation Names file in the Case of interest

## Import of Formation Names files

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

## Select the Formation file in a Case
To make the Formation information available for viewing, you have to select which of the Formation files to be used for a particular case.

![]({{ site.baseurl }}/images/formations_property_editor.PNG)

This option is available in the **Property Editor** for a case. The formation is selected in the combo box for property **Formation Names File**.

### Reload of formation data
If the formation file is modified outside ResInsight, the formation data can be imported again by the context menu **Formations->Reload**. This command will import formations for the selected formation files.

## Viewing the Formation Information

### Formations in 3D view
The formations can be visualized as a result property in **Cell Results**, **Cell Edge Result**, and **Separate Fault Result**. When selected, a special legend displaying formation names is activated.

### Property filter based on formations
Formation names are available in Property Filters as Result Type **Formation Names**. This makes it easy to filter geometry based on formation specifications.

See [ Cell Filters ]({{ site.baseurl }}/docs/filters) for details.

### Picking in 3D view
Picking on a cell being part of a formation will display the formation name in the **Result Info** windows, in addition to other pick info for the cell.

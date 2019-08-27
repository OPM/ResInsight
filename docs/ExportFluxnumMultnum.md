---
layout: docs
title: FLUXNUM/MULTNUM
permalink: /docs/exportfluxnummultnum/
published: true
---
The visible cells can be exported as a FLUXNUM or MULTNUM keyword that can be used in an Eclipse input data deck. 

You can do this by using the command **Export Visible Cells as FLUXNUM/MULTNUM** found by right clicking:
- **View** in the Project Tree.
- **Cell Result** in the Project Tree.
- In any Eclipse **3D View**.

The command can also be found in **File -> Export**. If the command is used in the project tree, the visible cells from the selected view are used for calculation. In the 3D view and from File -> Export, the visible cells from the currently active 3D view are used.

![]({{ site.baseurl }}/images/fluxnumMultnum.png)

- **Export Filename** -- Name of the file to export to.
- **Export Keyword** -- Selects the Eclipse keyword to export.
- **Visible Active Cells Value** -- This value is used for all the active cells that passes the filters(Range Filter, Property Filter etc) in the 3D View, and thus is visible. 
- **Hidden Active Cells Value** -- All the active cells that are not visible in the 3D view.
- **Inactive Cells Value** -- This value is used for all inactive cells, regardless of whether they are visible or not.

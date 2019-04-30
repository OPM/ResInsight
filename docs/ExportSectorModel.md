---
layout: docs
title: Export Sector Model
permalink: /docs/exportsectormodel/
published: true
---

Sub-sections of the Eclipse Grid with Parameters and Faults can be exported to Eclipse ASCII files in order to create new
Simulations on the sub-section.

### Exporting an Eclipse Sector Model

To launch the export dialog, right-click on either the **3D-view** in question or the **Cell Result**.

![]({{ site.baseurl }}/images/ExportSectorModel_RightClick.png) 

### Exporting Grid and Faults

![]({{ site.baseurl }}/images/ExportSectorModel_Grid.png) 

#### Grid Export

The Export dialog will allow the user to export grid data as ascii (An Eclipse Input Grid) to a specific file name by checking the **Export Grid Data** option.
If the option **Export in Local Coordinates** is checked, the UTM-portion of the coordinates will be stripped and the Grid will
be exported in a local coordinate system with no reference to actual location.

#### Grid Box Selection

The Grid Box selection group will allow the user to choose whether to export an IJK bounding box surrounding:
- **All Visible Cells** -- Controlled by range and property filters in the current view.
- **All Active Cells** -- All active cells in the Grid.
- **The Full Grid** -- The complete grid including inactive cells.
- **User Defined Selection** -- This will make the min and max IJK selection available to the user.

Furthermore, by checking **Make Invisible Cells Inactive** any cells that are within the IJK bounding box, but are invisible, will be made
inactive (ACTNUM = 0) in the exported grid.

#### Grid Refinement

The grid can be refined by a different integer in all three directions by changing the default value of Cell Count = 1 for I, J or K. The grid results will be not be interpolated but all new cells will inherit the value of their original cell.

#### Faults

Optionally export fault data to a separate fault file or append to the existing grid. Also, fault data can be ommitted by choosing "Do not Export" from the
**Export Fault Data** drop down list.

### Exporting Parameters

![]({{ site.baseurl }}/images/ExportSectorModel_Parameters.png) 

The Static result values in the Grid may be exported as Eclipse Input Parameters. The default parameters are 
EQLNUM, FIPNUM, NTG, PERMX, PERMY, PERMZ, PORO, PVTNUM, SATNUM and SWATINIT. ACTNUM is exported by default in the Grid Export file.

Other statuc result variables may be selected.

By default the Parameters will be exported to a separate file per parameter in the grid folder location. However it is possible to
append them to the grid file, export them all into a single parameter file or omit them completely be selecting different options in the **Export Parameters** drop down list.

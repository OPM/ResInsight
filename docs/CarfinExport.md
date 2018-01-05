---
layout: docs
title: CARFIN Export
permalink: /docs/carfinexport/
published: true
---

ResInsight can export a block of cells as an LGR by writing an Eclipse CARFIN keyword to a text file. 
To do this, right click an Eclipse-case in the  **Project Tree** and select the command **Export CARFIN...**.
The following dialog will open:

![]({{ site.baseurl }}/images/ExportCARFINDialog.png)

- **Source Case** -- The case to export from.
- **Export Filename** -- The file to write the CARFIN keyword to.
- **Source Grid Box** -- Options to control the box of cells defined by I, J, K and ranges to apply when exporting.
- **Grid Refinement** -- The LGR refinement options to use within the defined gridbox.
  - **Cell Count I,J,K** -- NX, NY and NZ in the CARFIN keyword.
  - **Max Well Count** -- The NWMAX in the CARFIN  keyword.

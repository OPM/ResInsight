---
layout: docs
title: Ensemble Plotting
permalink: /docs/ensembleplotting/
published: true
---
The current ResInsight version contains preview functionality for importing and plotting ensembles.

## Import
There are several ways to import an ensemble:
- Use the **Import Ensemble** command in the window menu
- Use the **Import Summary Case group** and then convert the group to an ensemble
- Import a number of summary cases, move them into a group and then convert the group to an ensemble

During import of a summary case, ResInsight tries to find an associated **parameters.txt** file containing ensemble parameters. If found, the ensemble parameters are stored together with the summary case. When the user adds a summary case to an ensemble, then ResInsight will perform ensemble parameters validation. A warning dialog is displayed in the following cases:
- One or more of the cases in the ensemble have no ensemble parameters
- The list of ensemble parameters differ between cases in the ensemble

## Plotting
Ensembles are plotted in the summary plot the same way as ordinary summary curves. Create a new summary plot using the [summary plot editor]({{site.baseurl}}/docs/summaryploteditor). Imported ensembles will appear in a separate group in the list of summary cases. When an ensemble is selected, a new curve set is created and plotted as multiple summary curves. By default all curves in an ensemble curve set will have the same color. The coloring mode may be edited in the curve set's property editor in the project plot tree view. Two coloring modes are available:
- **Single color** Use the same color for all curves in a curve set
- **By Ensemble Parameter** One ensemble parameter is selected to control coloring. The ensemble parameter value for each case is used to pick a color in a color range.


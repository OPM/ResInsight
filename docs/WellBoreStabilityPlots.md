---
layout: docs
title: Well Bore Stability Plots
permalink: /docs/wellborestabilityplots/
published: true
---
![]({{ site.baseurl }}/images/WellBoreStability.png)

ResInsight can create **Well Bore Stability** plots for Geomechanical cases. These plots are specialized [Well Log Plots]({{site.baseurl}}/docs/welllogandplots) and contain a visualization of [Formations]({{site.baseurl}}/docs/formations), [Well Path Attributes]({{site.baseurl}}/docs/wellpaths#well-path-attributes) as well as a set of well path derived curves in two different tracks. 

The third track contains curves showing different stability gradients (all normalized by mud weight):
- **FG**: Fracture Gradient for sands based on Kirsch.
- **OBG**: Overburden stress gradient: Stress component S_33.
- **PP**: Pore pressure.
- **SFG**: Shear Failure Gradient for shale based on Stassi-d'Alia.
- **SH**: Minimum horizontal stress.

The fourth track contains curves showing the angular orientation of the well path as azimuth (deviation from vertical) and inclination (deviation from x-axis) in degrees.

These plots can be created from the context menu for a well path in the 3D view or from the the context menu of the Well Log Plots entry in the 2D Plot Window. In the former case the well bore stability plot will be created for the selected **Well Path**. In the latter case it will be created for the first well path in the well path list and the well path for the entire plot can be changed with the [Change Data Source Feature]({{site.baseurl}}/docs/welllogandplots#change-data-source-for-plots-and-curves).

![]({{ site.baseurl }}/images/WellBoreStabilityCreation.png) ![]({{ site.baseurl }}/images/WellBoreStabilityCreation2.png)

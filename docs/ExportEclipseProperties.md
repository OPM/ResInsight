---
layout: docs
title: Export Eclipse Properties
permalink: /docs/exporteclipseproperties/
published: true
---

Eclipse Properties can be exported to Eclipse ASCII files. 
This is particularly useful when a new property is generated using Octave. 
The generated property can be exported for further use in the simulator.

### Export Command

To export the property currently active in the 3D View, activate the context menu on a **Cell Result** item in the **Project Tree**. 

![]({{ site.baseurl }}/images/ExportProperty.png) 

The following dialog will appear:

![]({{ site.baseurl }}/images/ExportPropertyDialog.png) 

- **Export File Name** -- The path to exported file
- **Eclipse Keyword** -- The keyword to use for the property in the eclipse file
- **Undefined Value** -- This value is written to the file for all values that are flagged as undefined in ResInsight

### File format

The exported file has the following format, that matches the Eclipse input format:

    -- Exported from ResInsight
    <keyword>
    <One number per cell separated by spaces>
    /


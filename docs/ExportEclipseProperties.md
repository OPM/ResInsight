---
layout: docs
title: Export Eclipse Properties
permalink: /docs/exporteclipseproperties/
published: true
---

Eclipse Properties can be exported to Eclipse ASCII files by activating the context 
menu on a **Cell Result** item in the **Project Tree**. 

![]({{ site.baseurl }}/images/ExportProperty.png) 

The command will export the property that currently is active in the 3D View. 

This is particularly useful when a new property is generated using Octave. 
The generated property can be exported for further use in the simulator.

The exported file has the following format, that matches the Eclipse input format:

    -- Exported from ResInsight
    <keyword>
    <One number per cell separated by spaces>
    /


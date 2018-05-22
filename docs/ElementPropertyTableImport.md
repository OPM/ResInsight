---
layout: docs
title: Element Property Table Import
permalink: /docs/elementpropertytableimport/
published: true
---

Element property tables in ABQUS input file format can be imported into ResInsight and displayed as Element Results. This can be used to display material properties, or any scalar value on each element.

To view the data as a **Color Result** select the **Result Position**: ***Element*** ( See [Geomechanical Results]({{ site.baseurl }}/docs/cellresults#geomechanical-results) )

A couple of property names are recognized and treated specially:
- MODULUS -- Scaled by 1.0e-9 and shown as "Yong's Modulus" in the user interface
- RATIO -- Shown as "Poisson's Ratio" in the user interface

### File Format

A couple of examples on the file format are shown below.

ResInsight searches for the first line containing `*Distribution Table`, then splits the following line by `,`. These entries describes the expected property values to be found in the file.

ResInsight then searches for the data block by ignoring lines 
- starting with `*` and `,` 
- does not have the expected column count when splitting the line by `,` 

When the datablock is found, the part of the line before `.` is stripped away, and first column is expected to be element ID

```
** ELASTIC SETTING FOR EACH ELEMENT
*Distribution Table, name=RSV_Res-1_Elastic_Table
MODULUS, RATIO
*Distribution, name=RSV_Res-1_ELASTICS, location=ELEMENT, Table=RSV_Res-1_Elastic_Table
** Description: Element-by-Element Elastic properties
,     2.
Res-1.210, 11198814808.2538, 0.19041
Res-1.209, 11207002032.1436, 0.19063
Res-1.208, 11222989223.0933, 0.19104
Res-1.207, 11202549454.349, 0.19051
```
```
** DENSITY SETTING FOR EACH ELEMENT
*Distribution Table, name=RSV_Res-1_Density_Table
Density
*Distribution, name=RSV_Res-1_DENSITIES, location=ELEMENT, Table=RSV_Res-1_Density_Table
** Description: Element-by-Element Densities
,     1.
Res-1.210, 2500
Res-1.209, 2500
Res-1.208, 2500
Res-1.207, 2500
```

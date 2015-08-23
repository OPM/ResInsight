---
layout: docs
title: ResInsight 1.4 Users Guide
next_section: odbsupport
permalink: /docs/home/
published: true
---

ResInsight is an open source, cross-platform 3D visualization and post processing tool for Eclipse reservoir models and simulations. It can also be configured to visualize geomechanical simulations from ABAQUS.

The system also constitutes a framework for further development and can be extended to support new data sources and visualization methods, e.g. additional solvers, seismic data, CSEM, geomechanics, and more.

### Efficient User Interface
The user interface is tailored for efficient interpretation of reservoir simulation data with specialized visualizations of properties, faults and wells. It enables easy handling of a large number of realizations and calculation of statistics. To be highly responsive,  ResInsight exploits multi-core CPUs and GPUs. 

### Octave Integration
Integration with GNU Octave enables powerful and flexible result manipulation and computations. Derived results can be returned to ResInsight for further handling and visualization. Eventually, derived and computed properties can be directly exported to Eclipse input formats  for further simulation cycles and parameter studies.

### Data support
The main input data is
`*.GRID` and `*.EGRID` files along with their `*.INIT` and restart files `*.XNNN` and `*.UNRST`. 
ResInsight also supports selected parts of Eclipse input files and can read grid 
information and corresponding cell property data sets.

ResInsight can also be built with support for Geomechanic models from ABAQUS in the `*.odb` file format. 

### About
ResInsight has been co-developed by [Statoil ASA](http://www.statoil.com/), [Ceetron Solutions AS](http://www.ceetronsolutions.com/), and [Ceetron AS](http://ceetron.com/) with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models.

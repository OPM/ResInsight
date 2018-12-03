---
layout: docs
title: ResInsight 2018.11
permalink: /docs/home/
published: true
---

ResInsight is an open source, cross-platform 3D visualization, curve plotting and post processing tool for Eclipse reservoir models and simulations. It can also be configured to visualize geomechanical simulations from ABAQUS.

The system also constitutes a framework for further development and can be extended to support new data sources and visualization methods, e.g. additional solvers, seismic data, CSEM, and more.

### Efficient User Interface
The user interface is tailored for efficient interpretation of reservoir simulation data with specialized visualizations of properties, faults and wells. It enables easy handling of a large number of realizations and calculation of statistics. To be highly responsive, ResInsight exploits multi-core CPUs and GPUs. Efficient plotting of well log plots and summary vectors is available through selected plotting features.

### Flow Diagnostics
Flow diagnostics calculations are embedded in the user interface and allows instant visualization of several well-based flow diagnostics properties, such as : Time of flight, flooding and drainage regions, well pair communication, well tracer fractions, well allocation plots and well communication lines. The calculations are performed by a library called [opm-flowdiagnostics](https://github.com/OPM/opm-flowdiagnostics) developed by [SINTEF Digital](http://www.sintef.no/sintef-ikt/#/). [More...]({{site.baseurl}}/docs/cellresults#method)

### Octave Integration
Integration with GNU Octave enables powerful and flexible result manipulation and computations. Derived results can be returned to ResInsight for further handling and visualization. Eventually, derived and computed properties can be directly exported to Eclipse input formats for further simulation cycles and parameter studies.

### Data Support
The main input data is
_`*.GRID`_ and _`*.EGRID`_ files along with their _`*.INIT`_ and restart files _`*.XNNN`_ and _`*.UNRST`_. 
Summary vectors can be imported from _`*.SMSPEC`_ files.
ResInsight also supports selected parts of Eclipse input files and can read grid 
information and corresponding cell property data sets from _`*.GRDECL`_ files. 
Well log data can be imported from _`*.LAS`_ files.

ResInsight can also be built with support for Geomechanical models from ABAQUS in the _`*.odb`_ file format.

### Updating and Refining Eclipse simulation models
ResInsight contains several pre-processing tools for updating and improving Eclipse reservoir models, including but not limited to:
- Adding **Well Path Completions** such as fractures, fishbones and perforations to well paths, including transmissibility calculations to allow for simulation in Eclipse.
- Easily and visually generate setup files for **Local Grid Refinement** (LGR)
- The generation of Eclipse **Multi Segment Well**-models for well path completions.

### About
ResInsight has been co-developed by [Equinor ASA](https://www.equinor.com/), [Ceetron Solutions AS](http://www.ceetronsolutions.com/), and [Ceetron AS](http://ceetron.com/) with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models.

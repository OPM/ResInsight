# ![](images/AppLogo48x48.png) ResInsight 1.0 Users Guide 

## Introduction

ResInsight is an open source, cross-platform 3D visualization and post processing tool for reservoir models and simulations. The system also constitutes a framework for further development and support for new data sources and visualization methods, e.g. additional solvers, seismic data, CSEM, geomechanics, and more. 

The user interface is tailored for efficient interpretation of reservoir simulation data with specialized visualizations of properties, faults and wells. It enables easy handling of a large number of realizations and calculation of statistics. To be highly responsive,  ResInsight exploits multi-core CPUs and GPUs. Integration with GNU Octave enables powerful and flexible result manipulation and computations. Derived results can be returned to ResInsight for further handling and visualization. Eventually, derived and computed properties can be directly exported to Eclipse input formats  for further simulation cycles and parameter studies.

The main input data is
`*.GRID` and `*.EGRID` files along with their `*.INIT` and restart files `*.XNNN` and `*.UNRST`. 
ResInsight also supports selected parts of Eclipse input files, often called `.grdecl`, and can read grid 
information and the corresponding cell property data sets.

ResInsight has been co-developed by Statoil, Ceetron Solutions AS, and Ceetron AS with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models.

### Contents

- [ Getting Started ](GettingStarted.md)
- [ Working with 3D Views ](ReservoirViews.md)
- [ Multiple realizations and statistics ](CaseGroupsAndStatistics.md)
- [ Octave Interface](OctaveInterface.md)
- [ Well Trajectories ](WellTrajectories.md)

### Appendix

- [ Octave Interface Reference](OctaveInterfaceReference.md)
- [ Regression Test System ](RegressionTestSystem.md)
- [ Command Line Arguments](CommandLineParameters.md)



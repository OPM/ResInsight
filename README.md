# ResInsight

ResInsight is an open source, cross-platform 3D visualization and post processing tool for reservoir models and simulations. The system also constitutes a framework for further development and support for new data sources and visualization methods, e.g. additional solvers, seismic data, CSEM, geomechanics, and more. 

The user interface is tailored for efficient interpretation of reservoir simulation data with specialized visualizations of properties, faults and wells. It enables easy handling of a large number of realizations and calculation of statistics. To be highly responsive, ResInsight exploits multi-core CPUs and GPUs. Integration with GNU Octave enables powerful and flexible result manipulation and computations. Derived results can be returned to ResInsight for further handling and visualization. Eventually, derived and computed properties can be directly exported to Eclipse input formats for further simulation cycles and parameter studies.

The main input data is *.GRID and *.EGRID files along with their *.INIT and restart files *.XNNN and *.UNRST. ResInsight also supports selected parts of Eclipse input files and can read grid information and corresponding cell property data sets.

ResInsight has been co-developed by Equinor ASA, Ceetron Solutions AS, and Ceetron AS with the aim to provide a versatile tool for professionals who need to visualize and process reservoir models. The software is copyrighted by Ceetron and Equinor and licensed under GPL 3+. See COPYING for details.

### Dependencies
ResInsight uses the Equinor/libecl (formerly Ensambles/ert) library to access Eclipse result files, and the two projects collaborates closely. The source code of the approved libecl version is embedded in the ResInsight source code tree, making downloading and building simple.
ResInsight also features an interface to Octave for retrieval of data from ResInsight, processing using Octave, and communication of data back into ResInsight for further handling and visualization.

Octave : [http://www.gnu.org/software/octave/](http://www.gnu.org/software/octave/)

Equinor/libecl : [https://github.com/Equinor/libecl](https://github.com/Equinor/libecl)

### Supported Platforms
ResInsight is designed to be cross-platform. Efforts have been made to ensure that code will compile and run on most Linux and Windows platforms. Currently, automated tests are run regularly on Red Hat Enterprise Linux (RHEL) 7, CentOS 7, Ubuntu 16.04, 18.04 and 20.04 as well as Windows 10 and Windows Server 2019.

### Documentation

See the [ ResInsight ](http://resinsight.org/) website and the [ Users Guide ](http://resinsight.org/docs/home/) for project documentation.

[ ResInsight Tutorials](https://github.com/CeetronSolutions/resinsight-tutorials)

### Source Code

    git clone git://github.com/OPM/ResInsight.git

### Contribution
Contributions are very welcome, although it might take some time for the team to accept pull requests that is not in the main line of the projects focus. 

Please use the dev branch for contributions and pull requests, as it is the branch dedicated to the day to day development. 

The master branch is supposed to be stable, and is updated when we want to publish a new stable release.

Release branches that might pop up are dedicated bug fix branches for the release in question.

### Building ResInsight

See [ Build Instructions ](https://resinsight.org/getting-started/download-and-install/buildinstructions/)

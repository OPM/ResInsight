# ResInsight

ResInsight is an open source, cross-platform 3D visualization and post-processing tool for reservoir models and simulations.

## Key Features

- **Specialized Visualizations**: Tailored for efficient interpretation of reservoir simulation data with visualizations of properties, faults, and wells
- **Performance-Oriented**: Exploits multi-core CPUs and GPUs for highly responsive operation
- **Statistical Analysis**: Handles large numbers of realizations with built-in statistical calculations
- **Python Integration**: Enables powerful result manipulation and computations with two-way data exchange
- **GNU Octave Integration**: Enables powerful result manipulation and computations with two-way data exchange
- **Extensible Framework**: Supports additional data sources and visualization methods (solvers, seismic data, CSEM, geomechanics)
- **Eclipse Integration**: Supports export to Eclipse input formats for simulation cycles and parameter studies

## Supported Input Formats

- Eclipse binary output (*.GRID, *.EGRID files with corresponding *.INIT, *.XNNN, and *.UNRST files)
- Selected Eclipse input file sections
- Grid information with cell property data sets

## Technology Stack

ResInsight utilizes:
- **Equinor/resdata** and **OPM/opm-common** libraries for Eclipse result file access
- **Qt** for the application framework
- **Qwt** for plotting functionality
- **vcpkg** for dependency management

## Platform Support

ResInsight is cross-platform with automated testing on:
- Red Hat Enterprise Linux (RHEL)
- Ubuntu
- Windows 11

## Documentation

- [ResInsight Website](http://resinsight.org/)
- [ResInsight Python API](http://api.resinsight.org/)
- [User Guide](http://resinsight.org/docs/home/)
- [Tutorials](https://github.com/CeetronSolutions/resinsight-tutorials)

## Development

### Source Code
```
git clone git://github.com/OPM/ResInsight.git
```
### Minimum Requirements
- gcc 13
- clang 19
- CMake 3.15
- MSVC 2022 17.4

These requirements are defined by used features in c++23 like `std::stacktrace` and `std::expected`.

### Dependencies
Most dependencies are managed using vcpkg as defined in [vcpkg.json](https://github.com/OPM/ResInsight/blob/dev/vcpkg.json)

### Contributing
Contributions are welcome! Please:
- Use the `dev` branch for contributions and pull requests
- Note that the `master` branch is kept stable and updated only for releases
- See [Spell Checking Guide](doc/spell-checking.md) for information on automated spell checking

### Building
See the [Build Instructions](https://resinsight.org/releases/build-from-source/build-instructions-ubuntu/) for detailed setup information.

## License

ResInsight is co-developed by Equinor ASA, Ceetron Solutions AS, and Ceetron AS. The software is copyrighted by Ceetron and Equinor and licensed under GPL 3+. See the COPYING file for details.

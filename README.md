# ResInsight

ResInsight is an open source, cross-platform 3D visualization and post-processing tool for reservoir models and simulations.

## Key Features

- **3D Visualization**: Grid properties, faults, LGRs, intersections, contour maps and cell filters, using multi-core CPUs for responsive operation
- **Ensembles**: Grid and summary ensembles with statistics, contour maps and correlation analysis
- **Plotting**: Summary, well log, RFT, PLT, well allocation, flow characteristics, cross plots and histograms
- **Wells and Completions**: Well paths, perforations, fishbones, valves and fractures, with export of completion data (COMPDAT, WELSEGS, COMPSEGS)
- **Geomechanics**: Abaqus results with derived stress and strain quantities
- **Seismic**: Seismic volumes displayed together with grids and wells
- **Model Export**: Sector models and grid properties in simulator input format, surfaces as IRAP and TSurf
- **Python API**: The `rips` package drives ResInsight and exchanges data through gRPC
- **GNU Octave**: Result manipulation and computations with two-way data exchange
- **Cloud Data**: Well paths and well logs from OSDU, and grid properties from Sumo

## Supported Input Formats

- **Grids and results**: Eclipse/OPM Flow binary output (`*.GRID`, `*.EGRID` with `*.INIT`, `*.UNRST`, `*.XNNN`), Eclipse input (`*.GRDECL`), ROFF (`*.roff`, `*.roffasc`)
- **Summary data**: Eclipse/OPM Flow summary (`*.SMSPEC`, `*.ESMRY`), observed data (`*.RSM`, `*.txt`, `*.csv`), Reveal and StimPlan summary (`*.csv`)
- **Well paths**: `*.dev`, `*.asc`, `*.ascii`, `*.rmswell`, `*.w`, `*.json`
- **Well logs**: LAS (`*.las`), CSV
- **Completions**: Perforation intervals (`*.ev`), valve templates, StimPlan and thermal fracture templates (`*.xml`, `*.csv`)
- **Surfaces**: IRAP (`*.irap`, `*.gri`), GOCAD TSurf (`*.ts`), VTK (`*.vtu`, `*.pvd`), point sets (`*.ptl`, `*.xyz`, `*.dat`)
- **Geomechanics**: Abaqus (`*.odb`, `*.inp`), VTK (`*.pvd`)
- **Seismic**: ZGY (`*.zgy`), OpenVDS (`*.vds`), SEG-Y (`*.sgy`, `*.segy`)
- **Other**: Formation names (`*.lyr`), polygons (`*.pol`, `*.csv`, `*.dat`), VFP tables (`*.vfp`, `*.ecl`), well measurements, pressure-depth data

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
- macOS (Apple Silicon and Intel)

## Documentation

- [ResInsight Website](http://resinsight.org/)
- [ResInsight Python API](http://api.resinsight.org/)
- [User Guide](http://resinsight.org/docs/home/)
- [Tutorials](https://github.com/CeetronSolutions/resinsight-tutorials)

## Development

### Source Code
```
git clone https://github.com/OPM/ResInsight.git
```
### Minimum Requirements
- gcc 13
- clang 19
- AppleClang 17 (Xcode 16)
- CMake 3.26
- MSVC 2022 17.4

These requirements are defined by used features in c++23 like `std::stacktrace` and `std::expected`.

### Dependencies
Most dependencies are managed using vcpkg as defined in [vcpkg.json](https://github.com/OPM/ResInsight/blob/dev/vcpkg.json)

### Contributing
Contributions are welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for the issue policy and pull request guidelines.

- Use the `dev` branch for contributions and pull requests
- Releases are defined by tags
- See [Spell Checking Guide](docs/spell-checking.md) for information on automated spell checking

### Building
See the [Build Instructions](https://resinsight.org/releases/build-from-source/build-instructions-ubuntu/) for detailed setup information.

## License

ResInsight is co-developed by Equinor ASA, Ceetron Solutions AS, and Ceetron AS. The software is copyrighted by Ceetron and Equinor and licensed under GPL 3+. See the COPYING file for details.

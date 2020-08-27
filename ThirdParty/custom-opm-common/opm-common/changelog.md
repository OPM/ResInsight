# Changelog

A short month-by-month synopsis of change highlights. Most bugfixes won't make
it in here, only the bigger features and interface changes.

# Important changes between release 2019.04 and 2019.10

* opm-common and the rest of OPM does not use libecl anymore and
  supports reading and writing Eclipse files directly
* Improved Eclipse compatible restart, support for unified and non unified
  files, and formatted and unformatted files
* Support for reading and checking various additional keywords was introduced (those
  starting with A - M, R, T, V, W, Z).
* ACTIONX support implemented
* NUPCOL support implemented
* UDA, UDQ support implemented
* Implemented writing saturation function scaled end-point arrays (e.g., SWL, SGU,
  SOWCR, KRORW, PCG) to INIT file
* Fixes concerning interaction of WELOPEN and WCON* with WECON and
  WTEST
* Added support for FOAM keywords (FOAMMOB, FOAMROCK, WFOAM)
* Refactored and reimplemented Well representation in deck

# 2016.12
* ZCORN adjustments improved, considers cell-cell relations
* Slightly more robust compilation - won't crash if locales are broken
* Accessing the PVTW table has a richer interface
* FAULTS face direction accepts X+, I+, Y+, J+, Z+ and K+
* WELOPEN can be controlled with completion numbers (last two parameters)
* COMPLUMP is now supported
* Don't crash on aquifer keywords
* GMWSET and FMWSET are expanded properly
* Don't crash on DEBUG
* Read support for COORDSYS, GRUPRIG, LGR, PRORDER, TRACERS, TUNINGDP,
  WDFACCOR, WEFAC, and WORKLIM, no longer crashes.
* RS and RV support.
* Support for DENSITY, PVTW, and ROCK tables
* JFUNC is understood and exposed

# 2016.11
* A new class, Runspec, for the RUNSPEC section, has been introduced
* Nodes in the FIELD group are no longer added to the Summary config
* WCONHIST only adds phases present in the deck
* cJSON can now be installed externally
* DeckItem and ParserItem internals refactored
* Build time reduced by only giving necessary source files to the json compiler
* Support for OPERATE, WSEGITER and GCONPROD
* Internal shared_ptrs removed from Schedule and children; interface updated
* Schedule is now copyable with regular C++ copy semantics - no internal refs
* Well head I/J is now time step dependent
* Well reference depth is time step dependent
* Some ZCORN issues fixed
* gas/oil and oil/gas ratio unit fixed for FIELD units

# 2016.10
* Significant improvements in overall parser performance
* shared_ptr has largely been removed from all public interfaces
* JFUNC keyword can be parsed
* Boolean conversions are explicit
* The Units.hpp header from core is moved here, replacing ConversionFactors
* The ConstPtr and Ptr shared pointer aliases are removed
* UnitSystem, Eclipse3DProperties, and OilVaporizationProperties are default
  constructible

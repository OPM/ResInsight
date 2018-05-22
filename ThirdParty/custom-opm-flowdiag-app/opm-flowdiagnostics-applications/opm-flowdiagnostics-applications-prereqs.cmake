# defines that must be present in config.h for our headers
set (opm-flowdiagnostics-applications_CONFIG_VAR
  )

# Build prerequisites
set (opm-flowdiagnostics-applications_DEPS
  # This module requires C++11 support, including std::regex
  "CXX11Features REQUIRED"
  # We need Boost.Filesystem for advanced file handling
  #   filesystem::path
  #   filesystem::directory_iterator
  #   filesystem::last_write_time()
  "Boost 1.44.0
    COMPONENTS filesystem system unit_test_framework REQUIRED"
  # We need LibECL to handle ECL result sets.
  "ecl REQUIRED"
  # Prerequisite OPM modules
  #   common -> Parameter System
  #   fdiag  -> Solver
  "opm-common REQUIRED"
  "opm-flowdiagnostics REQUIRED"
  )

find_package_deps(opm-flowdiagnostics-applications)

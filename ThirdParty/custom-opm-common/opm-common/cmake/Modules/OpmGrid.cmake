# - Recreate grid selection macros from DUNE
#
# If anyone requires Dune::GridSelector::GridType, they must call this
# macro in *their* project, to add this information to config.h. (In
# the autotools version, dunecontrol will automatically include m4
# scripts that does this).
#
# Example:
#	opm_cornerpoint_grid (${CONFIG_H})

include (CMakeParseArguments)
function (opm_grid_type)
  cmake_parse_arguments (a "" "FILENAME;SYMBOL;TYPE;CONDITION" "HEADERS" ${ARGN})

  # write prelude of a condition to use this particular grid, an inclusion guard,
  # and checks to see if the number of dimensions fits for this type of grid
  file (APPEND ${a_FILENAME}
"/* add GRIDTYPE typedef for grid implementation ${a_TYPE}:
    defining ${a_SYMBOL} during compilation typedefs this grid implementation as GridType
    in namespace Dune::GridSelector;
    also integer constants dimgrid and dimworld are set in this namespace.
    The required headers for this grid implementation are also included.
  */
#if defined ${a_SYMBOL} && ! defined USED_${a_SYMBOL}_GRIDTYPE
  /* someone else has already defined a gridtype */
  #if HAVE_GRIDTYPE
    #error \"Ambigious definition of GRIDTYPE\"
  #endif

  #ifndef WORLDDIM
    #define WORLDDIM GRIDDIM
  #endif
  #if not (WORLDDIM >= GRIDDIM)
    #error \"WORLDDIM < GRIDDIM does not make sense.\"
  #endif

  #if ! (${a_CONDITION})
    #error \"Preprocessor assertion ${a_CONDITION} failed.\"
  #endif

")

  # write headers which are capable of defining the type. this should
  # really just have consisted of a forward declaration, but the damage
  # is done: clients expect to just pull in config.h and have the
  # proper type available.
  foreach (header IN LISTS a_HEADERS)
	file (APPEND ${a_FILENAME}
	  "#include <${header}>\n"
	  )
  endforeach (header)

  # main part which does the typedef and then a postlude which marks
  # the grid as "taken" and make sure that no one else does the same
  file (APPEND ${a_FILENAME}
"
  namespace Dune {
    namespace GridSelector {
      const int dimgrid = GRIDDIM;
      const int worldgrid = WORLDDIM;
      typedef ${a_TYPE} GridType;
    }
  }

  #define HAVE_GRIDTYPE 1
  #define USED_${a_SYMBOL}_GRIDTYPE 1
#endif
")
  
endfunction (opm_grid_type)

# write the grid type for opm-grid
function (opm_cornerpoint_grid config_h)
  opm_grid_type (
	FILENAME ${CONFIG_H}
	SYMBOL CPGRID
	HEADERS "dune/grid/CpGrid.hpp" "dune/grid/cpgrid/dgfparser.hh"
	TYPE Dune::CpGrid
	CONDITION "(GRIDDIM == 3) && (WORLDDIM == 3)"
	)
endfunction (opm_cornerpoint_grid config_h)

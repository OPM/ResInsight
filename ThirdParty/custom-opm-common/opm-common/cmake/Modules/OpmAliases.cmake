# - Alias probed variables for compatibility with DUNE buildsystem
#
# DUNE build system sets some variables which have different names
# in the CMake modules we are using; this module set those variable
# so they can be exported to config.h visible to DUNE headers

function (set_aliases)
  # hardcoded list of "dune-var opm-var" pairs, where the components
  # are separated by space
  set (aliases
	"HAVE_UMFPACK             HAVE_SUITESPARSE_UMFPACK_H"
	"HAVE_DUNE_BOOST          HAVE_BOOST"
	)
  foreach (alias IN LISTS aliases)
	# convert entry "X Y" into a list "X;Y", then pick apart
	string (REGEX REPLACE "\ +" ";" tuple "${alias}")
	list (GET tuple 0 var)	
	list (GET tuple 1 name)

	# write this alias to cache
	set (${var} ${${name}} PARENT_SCOPE)
  endforeach (alias)
endfunction (set_aliases)

# always call this when the module is imported
set_aliases ()

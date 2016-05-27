# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-material_CONFIG_VAR
	HAVE_MPI
	HAVE_TYPE_TRAITS
	HAVE_VALGRIND
	HAVE_FINAL
	)

# dependencies
set (opm-material_DEPS
	# compile with C99 support if available
	"C99"
	# compile with C++0x/11 support if available
	"CXX11Features REQUIRED"
	# prerequisite OPM modules
	"opm-parser"
	"opm-common REQUIRED"
	# DUNE dependency
	"dune-common REQUIRED"
	)

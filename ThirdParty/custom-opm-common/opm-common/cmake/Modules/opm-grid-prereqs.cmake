# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-grid_CONFIG_VAR
	DUNE_GRID_VERSION_MAJOR
	DUNE_GRID_VERSION_MINOR
	DUNE_GRID_VERSION_REVISION
	DUNE_COMMON_VERSION_MAJOR
	DUNE_COMMON_VERSION_MINOR
	DUNE_COMMON_VERSION_REVISION
	HAVE_ZOLTAN
	)

# dependencies
set (opm-grid_DEPS
	# compile with C99 support if available
	"C99"
	# compile with C++0x/11 support if available
	"CXX11Features"
	# various runtime library enhancements
	"Boost 1.44.0
		COMPONENTS date_time filesystem system unit_test_framework REQUIRED"
	# DUNE dependency
	"dune-common REQUIRED;
	dune-grid REQUIRED;
	dune-geometry REQUIRED"
	# OPM dependency
        "opm-common REQUIRED;
        opm-core REQUIRED"
	"ZOLTAN"
	)

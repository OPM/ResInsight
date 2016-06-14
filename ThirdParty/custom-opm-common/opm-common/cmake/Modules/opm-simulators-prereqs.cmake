# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-simulators_CONFIG_VAR
	HAVE_OPM_GRID
	HAVE_PTHREAD
	)

# dependencies
set (opm-simulators_DEPS
	# Compile with C99 support if available
	"C99"
	# Compile with C++0x/11 support if available
	"CXX11Features"
	# Various runtime library enhancements
	"Boost 1.44.0
		COMPONENTS date_time filesystem system unit_test_framework REQUIRED"
	# DUNE prerequisites
	"dune-common REQUIRED;
	dune-istl REQUIRED"
	# OPM dependency
	"opm-common REQUIRED;
        opm-parser REQUIRED;
	opm-core REQUIRED;
	opm-output REQUIRED;
	opm-grid"
	# Eigen
	"Eigen3 3.2.0"
	)

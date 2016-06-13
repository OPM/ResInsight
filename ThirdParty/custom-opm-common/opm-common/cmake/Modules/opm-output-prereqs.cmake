# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-output_CONFIG_VAR
	HAVE_ERT
	)

# dependencies
set (opm-output_DEPS
	# compile with C99 support if available
	"C99"
	# compile with C++0x/11 support if available
	"CXX11Features REQUIRED"
	# various runtime library enhancements
	"Boost 1.44.0
		COMPONENTS filesystem system unit_test_framework REQUIRED"
	# Ensembles-based Reservoir Tools (ERT)
	"ERT REQUIRED"
	# Look for MPI support
	"opm-common REQUIRED"
	# Parser library for ECL-type simulation models
	"opm-parser REQUIRED"
	)

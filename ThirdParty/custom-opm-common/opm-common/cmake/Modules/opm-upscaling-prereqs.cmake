# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-upscaling_CONFIG_VAR
	HAVE_SUPERLU
	)

# dependencies
set (opm-upscaling_DEPS
	# compile with C99 support if available
	"C99"
	# compile with C++0x/11 support if available
	"CXX11Features"
	# various runtime library enhancements
	"Boost 1.44.0
		COMPONENTS date_time filesystem system iostreams unit_test_framework REQUIRED"
	# matrix library
	"BLAS REQUIRED"
	"LAPACK REQUIRED"
	# solver
	"SuperLU"
	# DUNE dependency
	"dune-common REQUIRED;
	dune-istl REQUIRED;
	dune-geometry REQUIRED;
	dune-grid REQUIRED;
	opm-common REQUIRED;
	opm-core REQUIRED;
        opm-grid REQUIRED;
	opm-output REQUIRED"
	)

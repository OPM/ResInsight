# -*- mode: cmake; tab-width: 2; indent-tabs-mode: t; truncate-lines: t; compile-command: "cmake -Wdev" -*-
# vim: set filetype=cmake autoindent tabstop=2 shiftwidth=2 noexpandtab softtabstop=2 nowrap:

# defines that must be present in config.h for our headers
set (opm-core_CONFIG_VAR
	HAVE_ERT
	HAVE_SUITESPARSE_UMFPACK_H
	HAVE_DUNE_ISTL
	HAVE_MPI
	HAVE_PETSC
	)

# dependencies
set (opm-core_DEPS
	# compile with C99 support if available
	"C99"
	# compile with C++0x/11 support if available
	"CXX11Features REQUIRED"
	# various runtime library enhancements
	"Boost 1.44.0
		COMPONENTS date_time filesystem system unit_test_framework REQUIRED"
	# matrix library
	"BLAS REQUIRED"
	"LAPACK REQUIRED"
	# Tim Davis' SuiteSparse archive
	"SuiteSparse COMPONENTS umfpack"
	# solver
	"SuperLU"
	# xml processing (for config parsing)
	"TinyXML"
	# Ensembles-based Reservoir Tools (ERT)
	"ERT REQUIRED"
	# Look for MPI support
	"MPI"
	# PETSc numerical backend
	"PETSc"
	# DUNE dependency
	"dune-common"
	"dune-istl"
	"opm-common REQUIRED"
	# Parser library for ECL-type simulation models
	"opm-parser REQUIRED"
	# the code which implements the material laws
	"opm-material REQUIRED"
	"opm-output REQUIRED"
	)

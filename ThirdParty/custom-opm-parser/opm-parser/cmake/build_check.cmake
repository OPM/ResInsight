# This file contains checks which are used to implement portable
# utility functions. The results of these check are assembled in the
# generated header "opm_parser_build_config.hpp" - that header is NOT part
# of the public api and it should only be included from source files
# as part of the compilation.

include( CheckFunctionExists )


check_function_exists( symlink OPM_PARSER_BUILD_HAVE_SYMLINK )


configure_file( ${PROJECT_SOURCE_DIR}/cmake/config/opm_parser_build_config.hpp.in ${PROJECT_BINARY_DIR}/opm_parser_build_config.hpp)
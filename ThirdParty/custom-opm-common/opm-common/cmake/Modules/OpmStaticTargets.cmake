####################################################################
#                                                                  #
# Setup static targets for all submodules.                         #
# Useful when building a static benchmark executable               #
#                                                                  #
####################################################################

# Macros

# Clone a git and build it statically
# If ARGN is specified installation is skipped, ARGN0 is
# a build-system target name and the rest of ARGN are build tool parameters
function(opm_from_git repo name revision)
  if(ARGN)
    list(GET ARGN 0 target)
    list(REMOVE_AT ARGN 0)
    # This is used for top build of benchmarks.
    # Clones the local source tree and builds it against the static libraries,
    # skipping the install step. Note that in pricinple URL instead of GIT_REPOSITORY
    # could have been used, but externalproject cannot handle build directories
    # which are a subdirectory of the source tree, and since that is typically the case
    # we work-around by re-cloning the local git.
    # The ommision of GIT_TAG ensures that we build the tip of the local git.
    set(COMMANDS BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target ${target} -- ${ARGN}
                 GIT_TAG ${revision}
                 INSTALL_COMMAND)
  else()
    # This is used with "normal" static builds.
    set(COMMANDS GIT_TAG ${revision})
  endif()
  externalproject_add(${name}-static
                      GIT_REPOSITORY ${repo}
                      PREFIX static/${name}
                      CONFIGURE_COMMAND PKG_CONFIG_PATH=${CMAKE_BINARY_DIR}/static/installed/lib/pkgconfig:${CMAKE_BINARY_DIR}/static/installed/${CMAKE_INSTALL_LIBDIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}
                                 ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/static/installed
                                 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                                 -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DBUILD_SHARED_LIBS=0
                                 -DBUILD_TESTING=0 -DBUILD_EXAMPLES=0 <SOURCE_DIR>
                                 -G ${CMAKE_GENERATOR}
                      ${COMMANDS} "")
  set_target_properties(${name}-static PROPERTIES EXCLUDE_FROM_ALL 1)
endfunction()

# Convenience macro for adding dependencies without having to include the -static all over
macro(opm_static_add_dependencies target)
  foreach(arg ${ARGN})
    add_dependencies(${target}-static ${arg}-static)
  endforeach()
endmacro()

include(ExternalProject)
include(GNUInstallDirs)

# Defaults to building master
if(NOT OPM_BENCHMARK_VERSION)
  set(OPM_BENCHMARK_VERSION "origin/master")
endif()

# ERT
externalproject_add(ert-static
                    GIT_REPOSITORY https://github.com/Ensembles/ert
                    PREFIX static/ert
                    GIT_TAG ${revision}
                    CONFIGURE_COMMAND ${CMAKE_COMMAND}
                                      -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/static/installed
                                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                      -DBUILD_SHARED_LIBS=0 <SOURCE_DIR>/devel)
set_target_properties(ert-static PROPERTIES EXCLUDE_FROM_ALL 1)

# 2015.04 release used dune v2.3.1
if(OPM_BENCHMARK_VERSION STREQUAL "release/2015.04/final")
  set(DUNE_VERSION v2.3.1)
endif()

# Master currently uses dune v2.3.1
if(OPM_BENCHMARK_VERSION STREQUAL "origin/master")
  set(DUNE_VERSION v2.3.1)
endif()

# Fallback
if(NOT DUNE_VERSION)
  set(DUNE_VERSION v2.3.1)
endif()

# Dune
foreach(dune_repo dune-common dune-geometry dune-grid dune-istl)
  opm_from_git(http://git.dune-project.org/repositories/${dune_repo} ${dune_repo} ${DUNE_VERSION})
endforeach()
opm_static_add_dependencies(dune-istl dune-common)
opm_static_add_dependencies(dune-geometry dune-common)
opm_static_add_dependencies(dune-grid dune-geometry)

# OPM
foreach(opm_repo opm-common opm-parser opm-core opm-output opm-grid opm-material
                 opm-upscaling)
  opm_from_git(https://github.com/OPM/${opm_repo} ${opm_repo} ${OPM_BENCHMARK_VERSION})
endforeach()
opm_static_add_dependencies(opm-parser opm-common ert)
opm_static_add_dependencies(opm-core opm-parser dune-istl)
opm_static_add_dependencies(opm-grid opm-core dune-grid)
opm_static_add_dependencies(opm-material opm-core)
opm_static_add_dependencies(opm-upscaling opm-grid opm-material)

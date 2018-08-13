project(libecl-ecl C CXX)



if (HAVE_BACKTRACE)
    list(APPEND opt_srcs util/util_abort_gnu.c)
else()
    list(APPEND opt_srcs util/util_abort_simple.c)
endif()

if (ERT_HAVE_GETUID AND ERT_HAVE_OPENDIR)
    list(APPEND opt_srcs util/test_work_area.cpp util/util_getuid.cpp)
endif()

if (ERT_HAVE_OPENDIR)
    list(APPEND opt_srcs util/util_opendir.cpp)
endif()

if (ERT_HAVE_SPAWN)
    list(APPEND opt_srcs util/util_spawn.cpp)
endif()

if (ERT_HAVE_LOCKF)
    list(APPEND opt_srcs util/util_lockf.cpp)
endif ()

if (ERT_HAVE_UNISTD)
   list(APPEND opt_srcs util/path_stack.cpp)
endif ()

if (MSVC)
    configure_file(include/ert/util/msvc_stdbool.h ert/util/stdbool.h)
endif ()

foreach (type int double long time_t float)
    set(TYPE ${type})
    set(SIGNED_TYPE true)
    configure_file(vector_template.h.in include/ert/util/${type}_vector.h)
    configure_file(vector_template.hpp.in include/ert/util/${type}_vector.hpp)
    configure_file(util/vector_template.cpp ${type}_vector.cpp)
    list(APPEND opt_srcs ${CMAKE_CURRENT_BINARY_DIR}/${type}_vector.cpp)
endforeach ()

foreach (type bool size_t)
    set(TYPE ${type})
    set(SIGNED_TYPE false)
    configure_file(vector_template.h.in include/ert/util/${type}_vector.h)
    configure_file(vector_template.hpp.in include/ert/util/${type}_vector.hpp)
    configure_file(util/vector_template.cpp ${type}_vector.cpp)
    list(APPEND opt_srcs ${CMAKE_CURRENT_BINARY_DIR}/${type}_vector.cpp)
endforeach ()

if (ZLIB_FOUND)
    list(APPEND opt_srcs util/util_zlib.cpp)
endif ()

if (ERT_BUILD_CXX)
    list(APPEND opt_srcs util/TestArea.cpp
                         ecl/FortIO.cpp
                         ecl/Smspec.cpp
                         ecl/EclFilename.cpp
)
endif ()

configure_file(build_config.h.in   include/ert/util/build_config.h)
configure_file(ert_api_config.h.in include/ert/util/ert_api_config.h)
configure_file(build_config.hpp.in   include/ert/util/build_config.hpp)
configure_file(ert_api_config.hpp.in include/ert/util/ert_api_config.hpp)

add_library(ecl util/rng.cpp
                util/lookup_table.cpp
                util/statistics.cpp
                util/mzran.cpp
                util/hash_node.cpp
                util/hash_sll.cpp
                util/hash.cpp
                util/node_data.cpp
                util/node_ctype.cpp
                util/util.c
                util/util_symlink.cpp
                util/util_lfs.c
                util/util_unlink.cpp
                util/arg_pack.cpp
                util/vector.cpp
                util/parser.cpp
                util/stringlist.cpp
                util/buffer.cpp
                util/timer.cpp
                util/string_util.cpp
                util/type_vector_functions.cpp
                util/ecl_version.cpp
                util/struct_vector.cpp
                util/perm_vector.cpp
                util/test_util.cpp
                ${opt_srcs}

                ecl/ecl_rsthead.cpp
                ecl/ecl_sum_tstep.cpp
                ecl/ecl_rst_file.cpp
                ecl/ecl_init_file.cpp
                ecl/ecl_grid_cache.cpp
                ecl/smspec_node.cpp
                ecl/ecl_kw_grdecl.cpp
                ecl/ecl_file_kw.cpp
                ecl/ecl_file_view.cpp
                ecl/ecl_grav.cpp
                ecl/ecl_grav_calc.cpp
                ecl/ecl_smspec.cpp
                ecl/ecl_unsmry_loader.cpp
                ecl/ecl_sum_data.cpp
                ecl/ecl_sum_file_data.cpp
                ecl/ecl_util.cpp
                ecl/ecl_kw.cpp
                ecl/ecl_sum.cpp
                ecl/ecl_sum_vector.cpp
                ecl/fortio.c
                ecl/ecl_rft_file.cpp
                ecl/ecl_rft_node.cpp
                ecl/ecl_rft_cell.cpp
                ecl/ecl_grid.cpp
                ecl/ecl_coarse_cell.cpp
                ecl/ecl_box.cpp
                ecl/ecl_io_config.cpp
                ecl/ecl_file.cpp
                ecl/ecl_region.cpp
                ecl/ecl_subsidence.cpp
                ecl/ecl_grid_dims.cpp
                ecl/grid_dims.cpp
                ecl/nnc_info.cpp
                ecl/ecl_grav_common.cpp
                ecl/nnc_vector.cpp
                ecl/ecl_nnc_export.cpp
                ecl/ecl_nnc_data.cpp
                ecl/ecl_nnc_geometry.cpp
                ecl/layer.cpp
                ecl/fault_block.cpp
                ecl/fault_block_layer.cpp
                ecl/ecl_type.cpp
                ecl/ecl_type_python.cpp
                ecl/well_state.cpp
                ecl/well_conn.cpp
                ecl/well_info.cpp
                ecl/well_ts.cpp
                ecl/well_conn_collection.cpp
                ecl/well_segment.cpp
                ecl/well_segment_collection.cpp
                ecl/well_branch_collection.cpp
                ecl/well_rseg_loader.cpp

                geometry/geo_surface.cpp
                geometry/geo_util.cpp
                geometry/geo_pointset.cpp
                geometry/geo_region.cpp
                geometry/geo_polygon.cpp
                geometry/geo_polygon_collection.cpp

                src/fortio.cpp
)

if (ERT_WINDOWS)
    set_target_properties(ecl PROPERTIES PREFIX "lib")
    if (MSVC)
        set_target_properties(ecl PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
endif ()

target_link_libraries(ecl PUBLIC ${m}
                                 ${dl}
                                 ${pthread}
                                 ${blas}
                                 ${zlib}
                                 ${shlwapi}
)

target_include_directories(ecl
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
           $<INSTALL_INTERFACE:include>
    PRIVATE ${ZLIB_INCLUDE_DIRS}
            util
            include
            e3
            ${CMAKE_CURRENT_SOURCE_DIR}/private-include
            ${CMAKE_CURRENT_BINARY_DIR}/include
)

target_compile_definitions(ecl PRIVATE
            -DGIT_COMMIT=${GIT_COMMIT}
            -DGIT_COMMIT_SHORT=${GIT_COMMIT_SHORT}
            -DECL_VERSION_MAJOR=${ECL_VERSION_MAJOR}
            -DECL_VERSION_MINOR=${ECL_VERSION_MINOR}
            -DECL_VERSION_MICRO=${ECL_VERSION_MICRO}
            $<$<BOOL:${BIG_ENDIAN}>:HOST_BIG_ENDIAN>
)

target_compile_options(ecl PUBLIC ${pthreadarg})


if (ERT_USE_OPENMP)
    target_compile_options(ecl PUBLIC ${OpenMP_CXX_FLAGS})
    set_property(TARGET ecl APPEND PROPERTY LINK_FLAGS ${OpenMP_CXX_FLAGS})
    target_link_libraries( ecl PUBLIC ${OpenMP_EXE_LINKER_FLAGS})
endif ()

set_target_properties(ecl PROPERTIES
                          VERSION ${ECL_VERSION_MAJOR}.${ECL_VERSION_MINOR}
                          SOVERSION ${ECL_VERSION_MAJOR})

install(TARGETS ecl
        EXPORT  ecl-config
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/
        DESTINATION include
        PATTERN *.h
)
install(DIRECTORY include/
        DESTINATION include
        PATTERN *.hpp EXCLUDE
)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include
        PATTERN *.h
)

if (ERT_BUILD_CXX)
    install(DIRECTORY include/
            DESTINATION include
            PATTERN *.hpp
)
endif ()

if (NOT BUILD_TESTS)
    return ()
endif ()

add_executable(ecl3-testsuite test/testsuite.cpp
                              test/fortio.cpp
)
target_include_directories(ecl3-testsuite
    PRIVATE e3
)
target_link_libraries(ecl3-testsuite catch2 ecl)

add_test(NAME ecl3 COMMAND ecl3-testsuite)

foreach (name   ert_util_alloc_file_components
                ert_util_approx_equal
                ert_util_arg_pack
                ert_util_before_after
                ert_util_binary_split
                ert_util_buffer
                ert_util_clamp
                ert_util_chdir
                ert_util_filename
                ert_util_hash_test
                ert_util_parent_path
                ert_util_realpath
                ert_util_relpath_test
                ert_util_rng
                ert_util_sscan_test
                ert_util_statistics
                ert_util_strcat_test
                ert_util_stringlist_test
                ert_util_string_util
                ert_util_strstr_int_format
                ert_util_type_vector_functions
                ert_util_vector_test
                ert_util_datetime
                ert_util_normal_path
                ert_util_mkdir_p
        )

    add_executable(${name} util/tests/${name}.cpp)
    target_link_libraries(${name} ecl)
    add_test(NAME ${name} COMMAND ${name})
endforeach ()

add_executable(ecl_smspec_node ecl/tests/ecl_smspec_node.cpp)
target_link_libraries( ecl_smspec_node ecl)
add_test(NAME ecl_smspec_node COMMAND ecl_smspec_node)


add_executable(ert_util_work_area util/tests/ert_util_work_area.cpp)
target_link_libraries(ert_util_work_area ecl)
add_test(NAME ert_util_work_area
    COMMAND ert_util_work_area  data2/file1
                                ${CMAKE_CURRENT_SOURCE_DIR}/util/tests/data2/file2
                                data2
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/util/tests
)

add_executable(ert_util_cwd_test util/tests/ert_util_cwd_test.cpp)
target_link_libraries(ert_util_cwd_test ecl)
add_test(NAME ert_util_cwd_test COMMAND ert_util_cwd_test ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ert_util_copy_file util/tests/ert_util_copy_file.cpp)
target_link_libraries(ert_util_copy_file ecl)
add_test(NAME ert_util_copy_file
        COMMAND ert_util_copy_file
        $<TARGET_FILE:ert_util_copy_file>)

add_executable(ert_util_file_readable util/tests/ert_util_file_readable.cpp)
target_link_libraries(ert_util_file_readable ecl)
add_test(NAME ert_util_file_readable COMMAND ert_util_file_readable)

add_executable(ert_util_path_stack_test util/tests/ert_util_path_stack_test.cpp)
target_link_libraries(ert_util_path_stack_test ecl)
add_test(NAME ert_util_path_stack_test COMMAND ert_util_path_stack_test
    ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


if (HAVE_BACKTRACE)
   add_executable(ert_util_abort_gnu_tests util/tests/ert_util_abort_gnu_tests.cpp)
   target_link_libraries(ert_util_abort_gnu_tests ecl)
   add_test(NAME ert_util_abort_gnu_tests COMMAND ert_util_abort_gnu_tests)

   add_executable(ert_util_addr2line util/tests/ert_util_addr2line.cpp)
   target_link_libraries(ert_util_addr2line ecl)
   add_test(NAME ert_util_addr2line COMMAND ert_util_addr2line)
endif()

if (HAVE_UTIL_ABORT_INTERCEPT)
   add_executable(ert_util_struct_vector util/tests/ert_util_struct_vector.cpp)
   target_link_libraries(ert_util_struct_vector ecl)
   add_test(NAME ert_util_struct_vector COMMAND ert_util_struct_vector)

   add_executable(ert_util_type_vector_test util/tests/ert_util_type_vector_test.cpp)
   target_link_libraries(ert_util_type_vector_test ecl)
   add_test(NAME ert_util_type_vector_test COMMAND ert_util_type_vector_test)
endif()

if (ERT_HAVE_SPAWN)
   add_executable(ert_util_spawn util/tests/ert_util_spawn.cpp)
   target_link_libraries(ert_util_spawn ecl)
   add_test(NAME ert_util_spawn COMMAND ert_util_spawn)
endif()

#
# ecl
#

foreach (name   ecl_alloc_cpgrid
                ecl_alloc_grid_dxv_dyv_dzv
                ecl_fault_block_layer
                ecl_util_path_access
                ecl_grid_add_nnc
                ecl_grid_copy
                ecl_grid_create
                ecl_grid_DEPTHZ
                ecl_grid_export
                ecl_grid_init_fwrite
                ecl_grid_reset_actnum
                ecl_init_file
                ecl_kw_cmp_string
                ecl_kw_equal
                ecl_kw_fread
                ecl_kw_grdecl
                ecl_kw_init
                ecl_nnc_geometry
                ecl_nnc_info_test
                ecl_nnc_vector
                ecl_rft_cell
                ecl_sum_alloc_resampled_test
                ecl_file_view
                test_ecl_file_index
                test_transactions
                ecl_rst_file
                ecl_sum_writer
                ecl_util_filenames
                ecl_util_make_date_no_shift
                ecl_util_month_range
                ecl_valid_basename
                test_ecl_nnc_data
                ecl_nnc_pair
                well_conn_collection
                well_branch_collection
                well_conn
                well_state
                well_segment
                well_segment_conn
                well_segment_collection
                ecl_file
        )
        add_executable(${name} ecl/tests/${name}.cpp)
        target_link_libraries(${name} ecl)
        add_test(NAME ${name} COMMAND ${name})
endforeach ()

add_executable(ecl_unsmry_loader ecl/tests/ecl_unsmry_loader_test.cpp)
target_link_libraries(ecl_unsmry_loader ecl)
target_include_directories(ecl_unsmry_loader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/private-include)
add_test(NAME ecl_unsmry_loader COMMAND ecl_unsmry_loader)

add_executable(ecl_sum_data_intermediate ecl/tests/ecl_sum_data_intermediate_test.cpp)
target_link_libraries(ecl_sum_data_intermediate ecl)
add_test(NAME ecl_sum_data_intermediate COMMAND ecl_sum_data_intermediate)

add_executable(ecl_grid_cell_contains ecl/tests/ecl_grid_cell_contains.cpp)
target_link_libraries(ecl_grid_cell_contains ecl)
add_test(NAME ecl_grid_cell_contains1 COMMAND ecl_grid_cell_contains)

if (HAVE_UTIL_ABORT_INTERCEPT)
   add_executable(ecl_grid_corner ecl/tests/ecl_grid_corner.cpp)
   target_link_libraries(ecl_grid_corner ecl)
   add_test(NAME ecl_grid_corner COMMAND ecl_grid_corner)

   add_executable(ecl_layer ecl/tests/ecl_layer.cpp)
   target_link_libraries(ecl_layer ecl)
   add_test(NAME ecl_layer COMMAND ecl_layer)
endif()

add_executable(ecl_get_num_cpu ecl/tests/ecl_get_num_cpu_test.cpp)
target_link_libraries(ecl_get_num_cpu ecl)
add_test(NAME ecl_get_num_cpu COMMAND ecl_get_num_cpu
            ${CMAKE_CURRENT_SOURCE_DIR}/ecl/tests/data/num_cpu1
            ${CMAKE_CURRENT_SOURCE_DIR}/ecl/tests/data/num_cpu2
            ${CMAKE_CURRENT_SOURCE_DIR}/ecl/tests/data/num_cpu3
            ${CMAKE_CURRENT_SOURCE_DIR}/ecl/tests/data/num_cpu4)

# The ecl_win64 application is not built as a proper test integrated
# into the CTEST system. Should be invoked manually on Windows.
if (ERT_WINDOWS)
   add_executable(ecl_lfs ecl/tests/ecl_lfs.cpp)
   target_link_libraries(ecl_lfs ecl)
endif()

#
# geometry
#


foreach (name geo_util_xlines geo_polygon geo_polygon_collection)
    add_executable(${name} geometry/tests/${name}.cpp)
    target_link_libraries(${name} ecl)
    add_test(NAME ${name} COMMAND ${name})
endforeach ()

if (ERT_BUILD_CXX)
    foreach (test ert_util_unique_ptr ert_util_test_area_xx)
        add_executable(${test} util/tests/${test}.cpp)
        target_link_libraries(${test} ecl)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()

    foreach (test eclxx_kw eclxx_fortio eclxx_smspec eclxx_filename eclxx_types)
        add_executable(${test} ecl/tests/${test}.cpp)
        target_link_libraries(${test} ecl)
        add_test(NAME ${test} COMMAND ${test})
    endforeach ()
endif ()


if (NOT STATOIL_TESTDATA_ROOT)
    return ()
endif()

#
# ecl
#


add_executable(ecl_coarse_test ecl/tests/ecl_coarse_test.cpp)
target_link_libraries(ecl_coarse_test ecl)
add_test(NAME ecl_coarse_test COMMAND ecl_coarse_test
    ${_eclpath}/LGCcase/LGC_TESTCASE2)

add_executable(ecl_grid_layer_contains ecl/tests/ecl_grid_layer_contains.cpp)
target_link_libraries(ecl_grid_layer_contains ecl)
add_test(NAME ecl_grid_layer_contains1 COMMAND ecl_grid_layer_contains
    ${_eclpath}/Gurbat/ECLIPSE.EGRID)
add_test(NAME ecl_grid_layer_contains2 COMMAND ecl_grid_layer_contains
    ${_eclpath}/Mariner/MARINER.EGRID)

add_executable(ecl_restart_test ecl/tests/ecl_restart_test.cpp)
target_link_libraries(ecl_restart_test ecl)
add_test(NAME ecl_restart_test COMMAND ecl_restart_test
    ${_eclpath}/Gurbat/ECLIPSE.UNRST)

add_executable(ecl_nnc_export ecl/tests/ecl_nnc_export.cpp)
target_link_libraries(ecl_nnc_export ecl)
add_test(NAME ecl_nnc_export1 COMMAND ecl_nnc_export ${_eclpath}/Gurbat/ECLIPSE TRUE)
add_test(NAME ecl_nnc_export2 COMMAND ecl_nnc_export ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC TRUE)
add_test(NAME ecl_nnc_export3 COMMAND ecl_nnc_export ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3 TRUE)
add_test(NAME ecl_nnc_export4 COMMAND ecl_nnc_export ${_eclpath}/DualPoro/DUAL_DIFF TRUE)
add_test(NAME ecl_nnc_export5 COMMAND ecl_nnc_export ${_eclpath}/DualPoro/DUALPORO TRUE)
add_test(NAME ecl_nnc_export6 COMMAND ecl_nnc_export ${_eclpath}/nestedLGRcase/TESTCASE_NESTEDLGR TRUE)
add_test(NAME ecl_nnc_export7 COMMAND ecl_nnc_export ${_eclpath}/TYRIHANS/BASE20150218_MULTFLT FALSE)

add_executable(ecl_nnc_export_get_tran ecl/tests/ecl_nnc_export_get_tran.cpp)
target_link_libraries(ecl_nnc_export_get_tran ecl)
add_test(NAME ecl_nnc_export_get_tran COMMAND ecl_nnc_export_get_tran
    ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3)

add_executable(ecl_nnc_data_statoil_root ecl/tests/test_ecl_nnc_data_statoil_root.cpp)
target_link_libraries(ecl_nnc_data_statoil_root ecl)
add_test(NAME ecl_nnc_data_statoil_root COMMAND ecl_nnc_data_statoil_root
    ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3
    ${_eclpath}/flow-nnc/Simple4/SIMPLE_SUMMARY4
    ${_eclpath}/flow-nnc/Gullfaks/GF_ACT_NEW_TEMP)

add_executable(ecl_util_make_date_shift ecl/tests/ecl_util_make_date_shift.cpp)
target_link_libraries(ecl_util_make_date_shift ecl)
add_test(NAME ecl_util_make_date_shift COMMAND ecl_util_make_date_shift)

add_executable(ecl_sum_case_exists ecl/tests/ecl_sum_case_exists.cpp)
target_link_libraries(ecl_sum_case_exists ecl)
add_test(NAME ecl_sum_case_exists COMMAND ecl_sum_case_exists
         ${_eclpath}/Gurbat/ECLIPSE
         ${_eclpath}/GurbatSummary/missingHeader/ECLIPSE
         ${_eclpath}/GurbatSummary/missingData/ECLIPSE)


add_executable(ecl_grid_lgr_name ecl/tests/ecl_grid_lgr_name.cpp)
target_link_libraries(ecl_grid_lgr_name ecl)
add_test(NAME ecl_grid_lgr_name COMMAND ecl_grid_lgr_name
    ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID)

add_executable(ecl_region ecl/tests/ecl_region.cpp)
target_link_libraries(ecl_region ecl)
add_test(NAME ecl_region COMMAND ecl_region ${_eclpath}/Gurbat/ECLIPSE.EGRID)

add_test(NAME ecl_grid_cell_contains2 COMMAND ecl_grid_cell_contains ${_eclpath}/Gurbat/ECLIPSE.EGRID)
add_test(NAME ecl_grid_cell_contains3 COMMAND ecl_grid_cell_contains ${_eclpath}/FF12/FF12_2013B2.EGRID)
add_test(NAME ecl_grid_cell_contains4 COMMAND ecl_grid_cell_contains ${_eclpath}/Brazil/R3_ICD.EGRID)

add_executable(ecl_grid_cell_contains_wellpath ecl/tests/ecl_grid_cell_contains_wellpath.cpp)
target_link_libraries(ecl_grid_cell_contains_wellpath ecl)
add_test(NAME ecl_grid_cell_contains_wellpath1
        COMMAND ecl_grid_cell_contains_wellpath
            ${_eclpath}/CellContains/model/SMS-0.EGRID
            ${_eclpath}/CellContains/R_PB-4H.jira)


add_executable(ecl_region2region ecl/tests/ecl_region2region_test.cpp)
target_link_libraries(ecl_region2region ecl)
add_test(NAME ecl_region2region COMMAND ecl_region2region ${_eclpath}/R2R/R2R.SMSPEC)

add_executable(ecl_grid_case ecl/tests/ecl_grid_case.cpp)
target_link_libraries(ecl_grid_case ecl)
add_test(NAME ecl_grid_case COMMAND ecl_grid_case
    ${_eclpath}/Gurbat/ECLIPSE.EGRID
    ${_eclpath}/Gurbat/ECLIPSE)

add_executable(ecl_lgr_test ecl/tests/ecl_lgr_test.cpp)
target_link_libraries(ecl_lgr_test ecl)
add_test(NAME ecl_lgr_test1 COMMAND ecl_lgr_test ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.EGRID)
add_test(NAME ecl_lgr_test2 COMMAND ecl_lgr_test ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.GRID)
add_test(NAME ecl_lgr_test3 COMMAND ecl_lgr_test ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID)

add_executable(ecl_grid_simple ecl/tests/ecl_grid_simple.cpp)
target_link_libraries(ecl_grid_simple ecl)
add_test(NAME ecl_grid_simple COMMAND ecl_grid_simple ${_eclpath}/Gurbat/ECLIPSE.EGRID)

add_test(NAME ecl_grid_ecl2015_2 COMMAND ecl_grid_simple
    ${_eclpath}/Eclipse2015_NNC_BUG/FF15_2015B2_LGRM_RDI15_HIST_RDIREAL1_20142.EGRID)

add_executable(ecl_grid_export_statoil ecl/tests/ecl_grid_export.cpp)
target_link_libraries(ecl_grid_export_statoil ecl)
add_test(NAME ecl_grid_export_statoil
    COMMAND ecl_grid_export_statoil ${_eclpath}/Gurbat/ECLIPSE.EGRID)

add_executable(ecl_grid_volume ecl/tests/ecl_grid_volume.cpp)
target_link_libraries(ecl_grid_volume ecl)
add_test(NAME ecl_grid_volume1 COMMAND ecl_grid_volume ${_eclpath}/Gurbat/ECLIPSE)
add_test(NAME ecl_grid_volume2 COMMAND ecl_grid_volume ${_eclpath}/VolumeTest/TEST1)
add_test(NAME ecl_grid_volume3 COMMAND ecl_grid_volume ${_eclpath}/OsebergSyd/Omega/OMEGA-0)
add_test(NAME ecl_grid_volume4 COMMAND ecl_grid_volume ${_eclpath}/Norne/reservoir_models/Norne_ATW2013/NORNE_ATW2013)

# The grid volume test fails miserably on the test case given as example five; looking at
# the failures one could actually suspect that the ECLIPSE algorithm for PORV calculations
# has been different in this file - i.e. that the absolute value of the individual
# tetrahedron parts have been taken during the sum, and not at the end. At least the ert
# algorithm gets volumes ~ 0 whereas ECLIPSE reports ~10^9 for the same cell.
# add_test( ecl_grid_volume5 ${EXECUTABLE_OUTPUT_PATH}/ecl_grid_volume ${_eclpath}/Heidrun/Summary/FF12_2013B3_CLEAN_RS)

add_executable(ecl_grid_dims ecl/tests/ecl_grid_dims.cpp)
target_link_libraries(ecl_grid_dims ecl)
add_test(NAME ecl_grid_dims0 COMMAND ecl_grid_dims)
add_test(NAME ecl_grid_dims1 COMMAND ecl_grid_dims ${_eclpath}/Gurbat/ECLIPSE.EGRID ${_eclpath}/Gurbat/ECLIPSE.INIT)
add_test(NAME ecl_grid_dims2 COMMAND ecl_grid_dims ${_eclpath}/Gurbat/ECLIPSE.GRID  ${_eclpath}/Gurbat/ECLIPSE.INIT)
add_test(NAME ecl_grid_dims3 COMMAND ecl_grid_dims ${_eclpath}/Gurbat/ECLIPSE.EGRID)
add_test(NAME ecl_grid_dims4 COMMAND ecl_grid_dims ${_eclpath}/Gurbat/ECLIPSE.GRID )
add_test(NAME ecl_grid_dims5 COMMAND ecl_grid_dims ${_eclpath}/AmalgLGRcase/TESTCASE_AMALG_LGR.EGRID)

add_executable(ecl_nnc_test ecl/tests/ecl_nnc_test.cpp)
target_link_libraries(ecl_nnc_test ecl)
add_test(NAME ecl_nnc_test1 COMMAND ecl_nnc_test ${_eclpath}/Gurbat/ECLIPSE.EGRID )
add_test(NAME ecl_nnc_test2 COMMAND ecl_nnc_test ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.EGRID )
add_test(NAME ecl_nnc_test3 COMMAND ecl_nnc_test ${_eclpath}/Troll/MSW_LGR/2BRANCHES-CCEWELLPATH-NEW-SCH-TUNED-AR3.EGRID)
add_test(NAME ecl_nnc_test4 COMMAND ecl_nnc_test ${_eclpath}/DualPoro/DUAL_DIFF.EGRID )
add_test(NAME ecl_nnc_test5 COMMAND ecl_nnc_test ${_eclpath}/nestedLGRcase/TESTCASE_NESTEDLGR.EGRID)

add_executable(ecl_layer_statoil ecl/tests/ecl_layer_statoil.cpp)
target_link_libraries(ecl_layer_statoil ecl)
add_test(NAME ecl_layer_statoil COMMAND ecl_layer_statoil
    ${_eclpath}/Mariner/MARINER.EGRID
    ${_eclpath}/Mariner/faultblock.grdecl)

add_executable(ecl_dualp ecl/tests/ecl_dualp.cpp)
target_link_libraries(ecl_dualp ecl)
add_test(NAME ecl_dualp COMMAND ecl_dualp ${_eclpath}/LGCcase/LGC_TESTCASE2)

add_executable(ecl_sum_test ecl/tests/ecl_sum_test.cpp)
target_link_libraries(ecl_sum_test ecl)
add_test(NAME ecl_sum_test COMMAND ecl_sum_test ${_eclpath}/Gurbat/ECLIPSE)

add_executable(ecl_sum_report_step_equal ecl/tests/ecl_sum_report_step_equal.cpp)
target_link_libraries(ecl_sum_report_step_equal ecl)
add_test(NAME ecl_sum_report_step_equal1 COMMAND ecl_sum_report_step_equal ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/Snorre/SNORRE FALSE)
add_test(NAME ecl_sum_report_step_equal2 COMMAND ecl_sum_report_step_equal ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/Gurbat/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_equal3 COMMAND ecl_sum_report_step_equal ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/extraMinistep/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_equal4 COMMAND ecl_sum_report_step_equal ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/short/ECLIPSE FALSE)
add_test(NAME ecl_sum_report_step_equal5 COMMAND ecl_sum_report_step_equal ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/enkf/ECLIPSE FALSE)
add_test(NAME ecl_sum_report_step_equal6 COMMAND ecl_sum_report_step_equal ${_eclpath}/Snorre/SNORRE  ${_eclpath}/Snorre2/SNORRE2 FALSE)

add_executable(ecl_sum_report_step_compatible ecl/tests/ecl_sum_report_step_compatible.cpp)
target_link_libraries(ecl_sum_report_step_compatible ecl)
add_test(NAME ecl_sum_report_step_compatible1 COMMAND ecl_sum_report_step_compatible ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/Snorre/SNORRE FALSE)
add_test(NAME ecl_sum_report_step_compatible2 COMMAND ecl_sum_report_step_compatible ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/Gurbat/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_compatible3 COMMAND ecl_sum_report_step_compatible ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/extraMinistep/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_compatible4 COMMAND ecl_sum_report_step_compatible ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/short/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_compatible5 COMMAND ecl_sum_report_step_compatible ${_eclpath}/Gurbat/ECLIPSE ${_eclpath}/modGurbat/enkf/ECLIPSE TRUE)
add_test(NAME ecl_sum_report_step_compatible6 COMMAND ecl_sum_report_step_equal      ${_eclpath}/Snorre/SNORRE  ${_eclpath}/Snorre2/SNORRE2 FALSE)

add_executable(ecl_file_statoil ecl/tests/ecl_file_statoil.cpp)
target_link_libraries(ecl_file_statoil ecl)
add_test(NAME ecl_file_statoil
         COMMAND ecl_file_statoil ${_eclpath}/Gurbat/ECLIPSE.UNRST ECLIPSE.UNRST)

add_executable(ecl_fmt ecl/tests/ecl_fmt.cpp)
target_link_libraries(ecl_fmt  ecl)
add_test(NAME ecl_fmt COMMAND ecl_fmt
    ${_eclpath}/Gurbat/ECLIPSE.UNRST
    ${_eclpath}/Gurbat/ECLIPSE.DATA)


add_executable(ecl_rsthead ecl/tests/ecl_rsthead.cpp)
target_link_libraries(ecl_rsthead ecl)
add_test(NAME ecl_rsthead COMMAND ecl_rsthead
    ${_eclpath}/Gurbat/ECLIPSE.UNRST
    ${_eclpath}/DualPoro/DUALPORO.X0005)

add_executable(ecl_smspec ecl/tests/ecl_smspec.cpp)
target_link_libraries(ecl_smspec ecl)
add_test(NAME ecl_smspec COMMAND ecl_smspec
    ${_eclpath}/Gurbat/ECLIPSE.SMSPEC
    ${_eclpath}/Heidrun/Summary/FF12_2013B3_CLEAN_RS.SMSPEC)


add_executable(ecl_rft ecl/tests/ecl_rft.cpp)
target_link_libraries(ecl_rft ecl)
add_test(NAME ecl_rft_rft    COMMAND ecl_rft ${_eclpath}/Gurbat/ECLIPSE.RFT RFT)
add_test(NAME ecl_rft_rft_rw COMMAND ecl_rft ${_eclpath}/Gurbat/ECLIPSE.RFT RFT_RW)
add_test(NAME ecl_rft_plt    COMMAND ecl_rft ${_eclpath}/RFT/TEST1_1A.RFT PLT)
add_test(NAME ecl_rft_mswplt COMMAND ecl_rft ${_eclpath}/RFT/RFT2.RFT MSW-PLT)
add_test(NAME ecl_rft_alloc  COMMAND ecl_rft ${_eclpath}/RFT/NORNE_ATW2013_RFTPLT_V2.RFT SIMPLE)

add_executable(ecl_grid_copy_statoil ecl/tests/ecl_grid_copy_statoil.cpp)
target_link_libraries(ecl_grid_copy_statoil ecl)

add_test(NAME ecl_grid_copy_statoil1 COMMAND ecl_grid_copy_statoil ${_eclpath}/Gurbat/ECLIPSE.EGRID)
add_test(NAME ecl_grid_copy_statoil2 COMMAND ecl_grid_copy_statoil ${_eclpath}/Mariner/MARINER.EGRID)
add_test(NAME ecl_grid_copy_statoil3 COMMAND ecl_grid_copy_statoil ${_eclpath}/LGCcase/LGC_TESTCASE2.EGRID)
add_test(NAME ecl_grid_copy_statoil4 COMMAND ecl_grid_copy_statoil ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.EGRID)

add_executable(ecl_fault_block_layer_statoil ecl/tests/ecl_fault_block_layer_statoil.cpp)
target_link_libraries(ecl_fault_block_layer_statoil ecl)
add_test(NAME ecl_fault_block_layer_statoil COMMAND ecl_fault_block_layer_statoil
    ${_eclpath}/Mariner/MARINER.EGRID
    ${_eclpath}/Mariner/faultblock.grdecl)

if (HAVE_UTIL_ABORT_INTERCEPT)
   add_executable(ecl_fortio ecl/tests/ecl_fortio.cpp)
   target_link_libraries( ecl_fortio ecl)
   add_test(NAME ecl_fortio COMMAND ecl_fortio ${_eclpath}/Gurbat/ECLIPSE.UNRST)
endif()

add_executable(well_state_load ecl/tests/well_state_load.cpp)
target_link_libraries( well_state_load ecl)

add_executable(well_state_load_missing_RSEG ecl/tests/well_state_load_missing_RSEG.cpp)
target_link_libraries(well_state_load_missing_RSEG ecl)

add_test(NAME well_state_load1 COMMAND well_state_load ${_eclpath}/Gurbat/ECLIPSE.EGRID
                                                       ${_eclpath}/Gurbat/ECLIPSE.X0030)
add_test(NAME well_state_load2 COMMAND well_state_load ${_eclpath}/MSWcase/MSW_CASE.EGRID
                                                       ${_eclpath}/MSWcase/MSW_CASE.X0021)
add_test(NAME well_state_load3 COMMAND well_state_load ${_eclpath}/Troll/MSW/MSW.EGRID
                                                       ${_eclpath}/Troll/MSW/MSW.X0123)
add_test(NAME well_state_load4 COMMAND well_state_load ${_eclpath}/Troll/MSW_LGR/LGR.EGRID
                                                       ${_eclpath}/Troll/MSW_LGR/LGR.X0095)
add_test(NAME well_state_load5 COMMAND well_state_load ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.EGRID
                                                       ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.X0061)
add_test(NAME well_state_load_missing_RSEG1
         COMMAND well_state_load_missing_RSEG ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.EGRID
                                              ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.X0061)
add_test(NAME well_state_load_missing_RSEG2
         COMMAND well_state_load_missing_RSEG ${_eclpath}/Troll/MSW/MSW.EGRID
                                              ${_eclpath}/Troll/MSW/MSW.X0123)

add_executable(well_segment_load ecl/tests/well_segment_load.cpp)
target_link_libraries(well_segment_load ecl)
add_test(NAME well_segment_load
        COMMAND well_segment_load ${_eclpath}/MSWcase/MSW_CASE.X0021)
 
add_executable(well_segment_branch_conn_load ecl/tests/well_segment_branch_conn_load.cpp)
target_link_libraries(well_segment_branch_conn_load ecl)
add_test(NAME well_segment_branch_conn_load
         COMMAND well_segment_branch_conn_load ${_eclpath}/MSWcase/MSW_CASE.X0021)

add_executable(well_info ecl/tests/well_info.cpp)
target_link_libraries(well_info ecl)
add_test(NAME well_info COMMAND well_info ${_eclpath}/Gurbat/ECLIPSE.EGRID)

add_executable(well_conn_CF ecl/tests/well_conn_CF.cpp)
target_link_libraries(well_conn_CF ecl)
add_test(NAME well_conn_CF COMMAND well_conn_CF ${_eclpath}/Gurbat/ECLIPSE.X0060)

add_executable(well_conn_load ecl/tests/well_conn_load.cpp)
target_link_libraries(well_conn_load ecl)
add_test(NAME well_conn_load1 COMMAND well_conn_load ${_eclpath}/Gurbat/ECLIPSE.X0030 F)
add_test(NAME well_conn_load2 COMMAND well_conn_load ${_eclpath}/10kcase/TEST10K_FLT_LGR_NNC.X0021 F)
add_test(NAME well_conn_load3 COMMAND well_conn_load ${_eclpath}/MSWcase/MSW_CASE.X0021 T)
add_test(NAME well_conn_load4 COMMAND well_conn_load ${_eclpath}/AmalgLGRcase/TESTCASE_AMALG_LGR.X0021 F)
add_test(NAME well_conn_load5 COMMAND well_conn_load ${_eclpath}/DualPoro/DUALPORO.X0009 F)
add_test(NAME well_conn_load6 COMMAND well_conn_load ${_eclpath}/0.9.2_LGR/BASE_REF_XY3Z1_T30_WI.X0003 F)

add_executable(well_ts ecl/tests/well_ts.cpp)
target_link_libraries(well_ts ecl)
add_test(NAME well_ts COMMAND well_ts ${_eclpath}/CO2case/BASE_CASE)

add_executable(well_dualp ecl/tests/well_dualp.cpp)
target_link_libraries(well_dualp ecl)
add_test(NAME well_dualp COMMAND well_dualp
    ${_eclpath}/Gurbat/ECLIPSE.UNRST
    ${_eclpath}/DualPoro/DUALPORO.X0005)

add_executable(well_lgr_load ecl/tests/well_lgr_load.cpp)
target_link_libraries(well_lgr_load ecl)

add_test(NAME well_lgr_load1 COMMAND well_lgr_load ${_eclpath}/0.9.2_LGR/BASE_REF_XY3Z1_T30_WI.EGRID
                                                   ${_eclpath}/0.9.2_LGR/BASE_REF_XY3Z1_T30_WI.X0003)
add_test(NAME well_lgr_load2 COMMAND well_lgr_load ${_eclpath}/AmalgLGRcase/TESTCASE_AMALG_LGR.EGRID
                                                   ${_eclpath}/AmalgLGRcase/TESTCASE_AMALG_LGR.X0016)

#
# geometry
#
add_executable(geo_surface geometry/tests/geo_surface.cpp)
target_link_libraries(geo_surface ecl)
add_test(NAME geo_surface COMMAND geo_surface
        ${_geopath}/Surface.irap
        ${_geopath}/Surface_incompatible.irap)

# ResInsight command-feature tests
#
# Separate executable from ResInsight-tests: the feature sweep executes
# Ric*Feature commands that require a RiaGuiApplication, so this target boots a
# full GUI application (offscreen on CI).

if(MSVC)
  # TARGET_RUNTIME_DLLS requires 3.21
  cmake_minimum_required(VERSION 3.21)
endif(MSVC)

set(SOURCE_FEATURETEST_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaFeatureTestModelBuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaFeatureTestModelBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaFeatureTestTreeView.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaFeatureTestTreeView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureSweepDenylist.h
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureExecutionRunner.h
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureExecutionRunner.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureSweep-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureExecutionSweep-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFeatureSubprocessSweep-Test.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/Tests/CMakeLists_files.cmake)

# googletest is fetched by the ApplicationLibCode/UnitTests subdirectory, which
# is added before this one. Only fetch it here if that has not happened (e.g.
# this subdirectory is used standalone).
if(NOT TARGET GTest::gtest)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
  )

  set(BUILD_GMOCK
      OFF
      CACHE BOOL "" FORCE
  )
  set(BUILD_GTEST
      ON
      CACHE BOOL "" FORCE
  )
  set(INSTALL_GTEST
      OFF
      CACHE BOOL "" FORCE
  )
  FetchContent_MakeAvailable(googletest)
endif()

# ##############################################################################
# Copy required Dll/.so to output folder
# ##############################################################################

foreach(riFileName ${RI_FILENAMES})
  list(
    APPEND
    copyFeatureTestCommands
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_if_different
    ${riFileName}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endforeach()
add_custom_target(PreBuildFileCopyFeatureTest ${copyFeatureTestCommands})
set_property(
  TARGET PreBuildFileCopyFeatureTest PROPERTY FOLDER "FileCopyTargetsTest"
)

qt_add_executable(ResInsight-featuretests ${SOURCE_FEATURETEST_FILES} main.cpp)

add_dependencies(ResInsight-featuretests PreBuildFileCopyFeatureTest)

target_include_directories(
  ResInsight-featuretests
  PUBLIC ${CMAKE_BINARY_DIR}/Generated
         "$<TARGET_PROPERTY:ApplicationLibCode,PUBLIC_INCLUDE_DIRECTORIES>"
         ${RI_PRIVATE_INCLUDES}
         ${PROJECT_SOURCE_DIR}/Commands
         ${PROJECT_SOURCE_DIR}/Commands/CompletionExportCommands
         ${CMAKE_CURRENT_LIST_DIR}
         ${CMAKE_CURRENT_LIST_DIR}/../TestModels
)

target_compile_features(ResInsight-featuretests PRIVATE cxx_std_20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND THIRD_PARTY_LIBRARIES rt atomic)
endif()

set(LINK_LIBRARIES
    ${THIRD_PARTY_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPM_LIBRARIES}
    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    ApplicationLibCode
    Commands
    RigGeoMechDataModel
    RifGeoMechFileInterface
)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND LINK_LIBRARIES GrpcInterface)
endif()

if(RESINSIGHT_USE_ODB_API)
  list(APPEND LINK_LIBRARIES RifOdbReader)
endif()

target_link_libraries(
  ResInsight-featuretests PUBLIC ${LINK_LIBRARIES} GTest::gtest
)

if(MSVC)
  add_custom_command(
    TARGET ResInsight-featuretests
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:ResInsight-featuretests>
      $<TARGET_FILE_DIR:ResInsight-featuretests>
    COMMAND_EXPAND_LISTS
  )

  set_target_properties(
    ResInsight-featuretests PROPERTIES COMPILE_FLAGS "/wd4190"
  )
endif(MSVC)

# The test executable boots with the "offscreen" QPA platform plugin (see
# main.cpp and the ctest ENVIRONMENT below). windeployqt only deploys the
# "windows" plugin, so copy the offscreen plugin next to the executable so it
# can be loaded at runtime on developer machines and CI.
if(TARGET Qt6::QOffscreenIntegrationPlugin)
  add_custom_command(
    TARGET ResInsight-featuretests
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:ResInsight-featuretests>/platforms
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:Qt6::QOffscreenIntegrationPlugin>
      $<TARGET_FILE_DIR:ResInsight-featuretests>/platforms/
    COMMENT
      "Deploying the Qt offscreen platform plugin for ResInsight-featuretests"
  )
endif()

add_test(NAME ResInsight-featuretests COMMAND ResInsight-featuretests)
set_tests_properties(
  ResInsight-featuretests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
                                     TIMEOUT 1800
)

# The execution sweep starts one child process per registered Ric*Feature, which
# takes minutes when run serially. Register it as a set of GoogleTest shards
# instead, so ctest -j runs them in parallel while every feature still gets its
# own child process and its own test case.
#
# Each shard is a full application process and each child is another one, so
# raise the shard count only as far as the machine has memory for. Measured on a
# 32 core machine: 4 shards 103 s, 8 shards 62 s, 20 shards 61 s. Going past 8
# buys no wall clock time but doubles the CPU spent, because the shards then
# become unbalanced. GoogleTest splits by test index while the cost per feature
# varies a lot, so one shard draws the expensive features and the run waits for
# it while the others sit idle.
set(RESINSIGHT_FEATURE_SWEEP_SHARDS
    8
    CACHE STRING
          "Number of parallel shards for the command feature execution sweep"
)

math(EXPR RI_LAST_SWEEP_SHARD "${RESINSIGHT_FEATURE_SWEEP_SHARDS} - 1")
foreach(shardIndex RANGE ${RI_LAST_SWEEP_SHARD})
  add_test(NAME ResInsight-featuresweep-${shardIndex}
           COMMAND ResInsight-featuretests --gtest_also_run_disabled_tests
                   --gtest_filter=*FeatureSubprocessExecutionTest*
  )
  set_tests_properties(
    ResInsight-featuresweep-${shardIndex}
    PROPERTIES
      ENVIRONMENT
      "QT_QPA_PLATFORM=offscreen;GTEST_TOTAL_SHARDS=${RESINSIGHT_FEATURE_SWEEP_SHARDS};GTEST_SHARD_INDEX=${shardIndex}"
      TIMEOUT
      1800
  )
endforeach()

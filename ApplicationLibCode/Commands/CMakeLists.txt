project(Commands)

# Unity Build
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
endif()

set(UNITY_EXCLUDE_FILES
    # Exclude moc files as they cause template instantiation issues
    # https://cmake.org/cmake/help/latest/prop_tgt/AUTOGEN_BUILD_DIR.html
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_autogen/mocs_compilation.cpp
)

message("Files excluded from UNITY_BUILD : ${UNITY_EXCLUDE_FILES}")

if(RESINSIGHT_ENABLE_UNITY_BUILD)
  foreach(fileToExclude ${UNITY_EXCLUDE_FILES})
    set_source_files_properties(
      ${fileToExclude} PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE
    )
  endforeach(fileToExclude)
endif()

set(COMMAND_REFERENCED_CMAKE_FILES
    CMakeLists_files.cmake
    AnalysisPlotCommands/CMakeLists_files.cmake
    ApplicationCommands/CMakeLists_files.cmake
    AnnotationCommands/CMakeLists_files.cmake
    CellFilterCommands/CMakeLists_files.cmake
    ColorLegendCommands/CMakeLists_files.cmake
    CompletionCommands/CMakeLists_files.cmake
    CompletionExportCommands/CMakeLists_files.cmake
    CorrelationPlotCommands/CMakeLists_files.cmake
    CrossSectionCommands/CMakeLists_files.cmake
    EclipseCommands/CMakeLists_files.cmake
    EclipseCommands/EclipseWell/CMakeLists_files.cmake
    EnsembleFileSetCommands/CMakeLists_files.cmake
    ExportCommands/CMakeLists_files.cmake
    FlowCommands/CMakeLists_files.cmake
    GridCrossPlotCommands/CMakeLists_files.cmake
    GeoMechCommands/CMakeLists_files.cmake
    JobCommands/CMakeLists_files.cmake
    HoloLensCommands/CMakeLists_files.cmake
    IntersectionBoxCommands/CMakeLists_files.cmake
    IntersectionViewCommands/CMakeLists_files.cmake
    MeasurementCommands/CMakeLists_files.cmake
    OctaveScriptCommands/CMakeLists_files.cmake
    OperationsUsingObjReferences/CMakeLists_files.cmake
    SummaryPlotCommands/CMakeLists_files.cmake
    OsduCommands/CMakeLists_files.cmake
    StreamlineCommands/CMakeLists_files.cmake
    SurfaceCommands/CMakeLists_files.cmake
    SeismicCommands/CMakeLists_files.cmake
    ToggleCommands/CMakeLists_files.cmake
    ViewLink/CMakeLists_files.cmake
    WellLogCommands/CMakeLists_files.cmake
    WellPathCommands/CMakeLists_files.cmake
    PlotTemplateCommands/CMakeLists_files.cmake
    FractureCommands/CMakeLists_files.cmake
    PlotBuilderCommands/CMakeLists_files.cmake
    PolygonCommands/CMakeLists_files.cmake
    Sumo/CMakeLists_files.cmake
    ToolCommands/CMakeLists_files.cmake
    3dView/CMakeLists_files.cmake
    Histogram/CMakeLists_files.cmake
)

# Include source file lists from *.cmake files
foreach(referencedfile ${COMMAND_REFERENCED_CMAKE_FILES})
  include(${referencedfile})
endforeach(referencedfile)

find_package(Eigen3 REQUIRED)

add_library(${PROJECT_NAME} OBJECT ${COMMAND_CODE_SOURCE_FILES})

# NOTE: USE_OPENVDS/USE_ODB_API only affect a handful of files in this
# target. Scope the defines to just those files via
# set_source_files_properties() rather than target_compile_definitions()
# (which would force a rebuild of every file in this target when toggled).
if(RESINSIGHT_ENABLE_OPENVDS)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/SeismicCommands/RicImportSeismicFeature.cpp
    PROPERTIES COMPILE_DEFINITIONS USE_OPENVDS
  )
endif()

if(RESINSIGHT_USE_ODB_API)
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMechCommands/RicGeoMechCopyCaseFeature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMechCommands/RicImportGeoMechCaseFeature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/GeoMechCommands/RicImportGeoMechCaseTimeStepFilterFeature.cpp
    PROPERTIES COMPILE_DEFINITIONS USE_ODB_API
  )
endif()

# RicHelpFeatures.cpp references both USE_ODB_API and USE_HDF5.
if(RESINSIGHT_USE_ODB_API OR RESINSIGHT_FOUND_HDF5)
  set(RIC_HELP_FEATURES_DEFS "")
  if(RESINSIGHT_USE_ODB_API)
    list(APPEND RIC_HELP_FEATURES_DEFS USE_ODB_API)
  endif()
  if(RESINSIGHT_FOUND_HDF5)
    list(APPEND RIC_HELP_FEATURES_DEFS USE_HDF5)
  endif()
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/ApplicationCommands/RicHelpFeatures.cpp
    PROPERTIES COMPILE_DEFINITIONS "${RIC_HELP_FEATURES_DEFS}"
  )
endif()

if(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
  message("Precompiled Headers is enabled on : ${PROJECT_NAME}")
  target_precompile_headers(Commands PRIVATE pch.h)
endif()

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_SOURCE_DIR}/EclipseCommands
          ${CMAKE_CURRENT_SOURCE_DIR}/CompletionExportCommands
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    Commands
    PRIVATE -Wall
            -Wno-reorder
            -Wno-switch
            -Wno-undefined-var-template
            -Wno-delete-abstract-non-virtual-dtor
            -Wno-call-to-pure-virtual-from-ctor-dtor
            -Wno-delete-non-abstract-non-virtual-dtor
            -Wno-ambiguous-reversed-operator
            -Wno-deprecated-declarations
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(Commands PRIVATE -Wno-deprecated-declarations)
endif()

if(MSVC)
  # The following warnings are supposed to be used in ResInsight, but
  # temporarily disabled to avoid too much noise warning C4245: 'return':
  # conversion from 'int' to 'size_t', signed/unsigned mismatch warning C4005:
  # Macro redefinition for math constants (M_PI, M_SQRT2 etc)

  # If possible, the following command is supposed to be the final target
  # set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /wd4190
  # /wd4100 /wd4127")

  set(BUILD_FLAGS_FOR_MSVC "/W3 /wd4190 /wd4100 /wd4127")

  if(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 19.38)
    # https://github.com/OPM/ResInsight/issues/10844
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4996")
  endif()

  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS_FOR_MSVC}
  )

endif()

set(LINK_LIBRARIES ApplicationLibCode ResultStatisticsCache
                   ResInsightCommonSettings
)
target_link_libraries(Commands PRIVATE ${LINK_LIBRARIES})

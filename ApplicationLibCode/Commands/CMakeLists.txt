project(Commands)

# Unity Build
if(RESINSIGHT_ENABLE_UNITY_BUILD)
  message("Cmake Unity build is enabled on : ${PROJECT_NAME}")
  set(CMAKE_UNITY_BUILD true)
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
    ExportCommands/CMakeLists_files.cmake
    FlowCommands/CMakeLists_files.cmake
    GridCrossPlotCommands/CMakeLists_files.cmake
    GeoMechCommands/CMakeLists_files.cmake
    HoloLensCommands/CMakeLists_files.cmake
    IntersectionBoxCommands/CMakeLists_files.cmake
    IntersectionViewCommands/CMakeLists_files.cmake
    MeasurementCommands/CMakeLists_files.cmake
    OctaveScriptCommands/CMakeLists_files.cmake
    OperationsUsingObjReferences/CMakeLists_files.cmake
    SummaryPlotCommands/CMakeLists_files.cmake
    OsduImportCommands/CMakeLists_files.cmake
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
)

# Include source file lists from *.cmake files
foreach(referencedfile ${COMMAND_REFERENCED_CMAKE_FILES})
  include(${referencedfile})
endforeach(referencedfile)

find_package(Eigen3 REQUIRED)

# Prefix files with COMMAND_ to avoid clash with application global lists
qt5_wrap_cpp(COMMAND_MOC_SOURCE_FILES ${COMMAND_QT_MOC_HEADERS})

add_library(
  ${PROJECT_NAME} OBJECT
  ${COMMAND_CODE_SOURCE_FILES} ${COMMAND_CODE_HEADER_FILES}
  ${COMMAND_MOC_SOURCE_FILES}
)

if(RESINSIGHT_ENABLE_PRECOMPILED_HEADERS)
  message("Precompiled Headers is enabled on : ${PROJECT_NAME}")
  target_precompile_headers(Commands PRIVATE pch.h)
endif()

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                          ${CMAKE_CURRENT_SOURCE_DIR}/EclipseCommands
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
  )
endif()

if(RESINSIGHT_TREAT_WARNINGS_AS_ERRORS)
  if(MSVC)
    target_compile_options(Commands PRIVATE /WX)
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_options(Commands PRIVATE -Werror)
  endif()
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

  if(CMAKE_CXX_COMPILER_VERSION LESS_EQUAL 19.14)
    # The following warning is generated over 800 times from a qwt header only
    # using VS2015 Disabling temporarily warning C4505 'function' : unreferenced
    # local function has been removed
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4505")
  endif()

  if(CMAKE_CXX_COMPILER_VERSION GREATER_EQUAL 19.38)
    # https://github.com/OPM/ResInsight/issues/10844
    set(BUILD_FLAGS_FOR_MSVC "${BUILD_FLAGS_FOR_MSVC} /wd4996")
  endif()

  set_target_properties(
    ${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${BUILD_FLAGS_FOR_MSVC}
  )

endif()

set(LINK_LIBRARIES ApplicationLibCode ResultStatisticsCache)

target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES})

source_group("" FILES ${PROJECT_FILES})

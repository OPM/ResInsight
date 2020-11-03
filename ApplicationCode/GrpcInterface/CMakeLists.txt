cmake_minimum_required(VERSION 3.12)

project(GrpcInterface)

message(STATUS "GRPC enabled")
add_definitions(-DENABLE_GRPC)

if(VCPKG_AUTO_INSTALL)
  vcpkg_install(grpc)
endif()

set(CMAKE_UNITY_BUILD false)
set(RESINSIGHT_GRPC_PYTHON_EXECUTABLE
    ""
    CACHE
      FILEPATH
      "gRPC : Path to Python 3 executable, required to build the Python client library"
)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x600)
endif()

add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.inl
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCaseService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcSimulationWellService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcAppService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPropertiesService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcNNCPropertiesService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPdmObjectService.h)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCaseService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcSimulationWellService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcAppService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPropertiesService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcNNCPropertiesService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPdmObjectService.cpp)

# Find Protobuf installation Looks for protobuf-config.cmake file installed by
# Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE ON)
find_package(Protobuf CONFIG 3.0 QUIET)

if(Protobuf_FOUND)
  message(STATUS "Using protobuf ${protobuf_VERSION}")

  # Find gRPC installation Looks for gRPCConfig.cmake file installed by gRPC's
  # cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Using gRPC ${gRPC_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

  set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure gRPC::grpc_unsecure gRPC::gpr)
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  set(GRPC_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

  set_target_properties(
    ${GRPC_LIBRARIES} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
                                 MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
else()
  set(RESINSIGHT_GRPC_INSTALL_PREFIX
      ""
      CACHE PATH "gRPC : Install prefix for gRPC")
  if(NOT DEFINED RESINSIGHT_GRPC_INSTALL_PREFIX
     OR NOT EXISTS ${RESINSIGHT_GRPC_INSTALL_PREFIX})
    message(
      FATAL_ERROR
        "You need a valid RESINSIGHT_GRPC_INSTALL_PREFIX set to build with gRPC"
    )
  endif()
  set(ENV{PKG_CONFIG_PATH} "${RESINSIGHT_GRPC_INSTALL_PREFIX}/lib/pkgconfig")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(GRPC REQUIRED grpc++_unsecure>=1.20 grpc_unsecure gpr
                    protobuf)
  if(EXISTS "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/protoc")
    set(_PROTOBUF_PROTOC "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/protoc")
  elseif(EXISTS "${RESINSIGHT_GRPC_INSTALL_PREFIX}/tools/protobuf/protoc")
    set(_PROTOBUF_PROTOC
        "${RESINSIGHT_GRPC_INSTALL_PREFIX}/tools/protobuf/protoc")
  else()
    message(FATAL_ERROR "Could not find the protobuf compiler (protoc)")
  endif()

  if(EXISTS "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE
        "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
  elseif(EXISTS "${RESINSIGHT_GRPC_INSTALL_PREFIX}/tools/grpc/grpc_cpp_plugin")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE
        "${RESINSIGHT_GRPC_INSTALL_PREFIX}/tools/grpc/grpc_cpp_plugin")
  endif()
  include_directories(AFTER ${GRPC_INCLUDE_DIRS})
endif()

set(LINK_LIBRARIES
    ${QT_LIBRARIES}
    ${GRPC_LIBRARIES}
    LibCore
    cafCommand
    cafPdmCvf
    cafPdmScripting
    cafTensor
    cafViewer
    cafVizExtensions
    ecl
    nightcharts
    qwt)

# Proto files
set(PROTO_FILES
    "Definitions"
    "PdmObject"
    "Case"
    "SimulationWell"
    "Project"
    "Commands"
    "NNCProperties"
    "App"
    "Properties"
    "Grid")

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/Python")

add_custom_target(PipInstall)

foreach(proto_file ${PROTO_FILES})
  get_filename_component(
    rips_proto "${CMAKE_CURRENT_LIST_DIR}/GrpcProtos/${proto_file}.proto"
    ABSOLUTE)
  get_filename_component(rips_proto_path "${rips_proto}" PATH)

  list(APPEND GRPC_PROTO_FILES_FULL_PATH ${rips_proto})

  set(rips_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
  set(rips_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
  set(rips_grpc_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
  set(rips_grpc_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")

  add_custom_command(
    OUTPUT "${rips_proto_srcs}" "${rips_proto_hdrs}" "${rips_grpc_srcs}"
           "${rips_grpc_hdrs}"
    COMMAND
      ${_PROTOBUF_PROTOC} ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated"
      --cpp_out "${CMAKE_BINARY_DIR}/Generated" -I "${rips_proto_path}"
      --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}" "${rips_proto}"
    DEPENDS "${rips_proto}")

  set(rips_proto_python "rips/generated/${proto_file}_pb2.py")
  set(rips_grpc_python "rips/generated/${proto_file}_pb2_grpc.py")

  if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
    message(
      STATUS "Using Python Executable: ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE}")
    add_custom_command(
      OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${rips_proto_python}"
             "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}"
      COMMAND
        ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m grpc_tools.protoc -I
        "${rips_proto_path}" --python_out
        "${GRPC_PYTHON_SOURCE_PATH}/rips/generated" --grpc_python_out
        "${GRPC_PYTHON_SOURCE_PATH}/rips/generated" "${rips_proto}"
      DEPENDS "${rips_proto}"
      COMMENT "Generating ${rips_proto_python} and ${rips_grpc_python}"
      VERBATIM)
    list(APPEND GRPC_PYTHON_GENERATED_SOURCES ${rips_proto_python}
         ${rips_grpc_python})
  else()
    message(
      STATUS
        "RESINSIGHT_GRPC_PYTHON_EXECUTABLE not specified. Will not install Python client code"
    )
  endif()

  list(APPEND GRPC_HEADER_FILES ${rips_proto_hdrs} ${rips_grpc_hdrs})

  list(APPEND GRPC_CPP_SOURCES ${rips_proto_srcs} ${rips_grpc_srcs})

endforeach(proto_file)

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  configure_file(${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.py.cmake
                 ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/RiaVersionInfo.py)
  configure_file(${GRPC_PYTHON_SOURCE_PATH}/setup.py.cmake
                 ${GRPC_PYTHON_SOURCE_PATH}/setup.py)
endif()

list(
  APPEND
  GRPC_PYTHON_SOURCES
  "rips/generated/RiaVersionInfo.py"
  "rips/__init__.py"
  "rips/case.py"
  "rips/contour_map.py"
  "rips/grid.py"
  "rips/gridcasegroup.py"
  "rips/instance.py"
  "rips/pdmobject.py"
  "rips/plot.py"
  "rips/project.py"
  "rips/simulation_well.py"
  "rips/view.py"
  "rips/well_log_plot.py"
  "rips/PythonExamples/instance_example.py"
  "rips/PythonExamples/command_example.py"
  "rips/PythonExamples/case_grid_group.py"
  "rips/PythonExamples/case_info_streaming_example.py"
  "rips/PythonExamples/create_wbs_plot.py"
  "rips/PythonExamples/export_plots.py"
  "rips/PythonExamples/export_snapshots.py"
  "rips/PythonExamples/error_handling.py"
  "rips/PythonExamples/import_well_paths_and_logs.py"
  "rips/PythonExamples/soil_porv_async.py"
  "rips/PythonExamples/soil_porv_sync.py"
  "rips/PythonExamples/selected_cases.py"
  "rips/PythonExamples/all_cases.py"
  "rips/PythonExamples/replace_case.py"
  "rips/PythonExamples/set_grid_properties.py"
  "rips/PythonExamples/set_cell_result.py"
  "rips/PythonExamples/set_flow_diagnostics_result.py"
  "rips/PythonExamples/grid_information.py"
  "rips/PythonExamples/input_prop_test_sync.py"
  "rips/PythonExamples/input_prop_test_async.py"
  "rips/PythonExamples/soil_average_async.py"
  "rips/PythonExamples/soil_average_sync.py"
  "rips/PythonExamples/view_example.py"
  "rips/tests/test_cases.py"
  "rips/tests/test_grids.py"
  "rips/tests/test_properties.py"
  "rips/tests/test_project.py"
  "rips/tests/conftest.py"
  "rips/tests/dataroot.py"
  "rips/tests/test_nnc_properties.py"
  "rips/tests/test_simulation_wells.py"
  "rips/tests/test_summary_cases.py"
  "rips/tests/test_wells.py"
  "requirements.txt"
  "setup.py"
  "README.md"
  "LICENSE")

list(APPEND GRPC_PYTHON_SOURCES ${GRPC_PYTHON_GENERATED_SOURCES})

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  foreach(PYTHON_SCRIPT ${GRPC_PYTHON_SOURCES})
    list(APPEND GRPC_PYTHON_SOURCES_FULL_PATH
         "${GRPC_PYTHON_SOURCE_PATH}/${PYTHON_SCRIPT}")
  endforeach()
endif()

add_library(
  ${PROJECT_NAME} OBJECT
  ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
  ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES})

if(RESINSIGHT_GRPC_INSTALL_PREFIX)
  target_include_directories(${PROJECT_NAME} PRIVATE ${GRPC_INCLUDE_DIRS})
endif()
target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})

if(MSVC)
  # GRPC generates a lot of harmless warnings on MSVC
  set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES COMPILE_FLAGS "/wd4251 /wd4702 /wd4005 /wd4244 /wd4125 /wd4267")
else()
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS
                                                   "-Wno-overloaded-virtual")
endif()

# install gRPC Python files
if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE)
  message(STATUS "Installing Python modules")
  add_custom_command(
    TARGET PipInstall
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install --upgrade
            pip
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install --user
            wheel setuptools pytest
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install --user
            grpcio-tools)
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE)
  message(STATUS "Bundling Python GRPC modules")
  add_custom_command(
    TARGET PipInstall
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install --user
            --target=${GRPC_PYTHON_SOURCE_PATH} grpcio-tools)
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  add_custom_target(GeneratedPythonSources ALL
                    DEPENDS ${GRPC_PYTHON_SOURCES_FULL_PATH})
  add_dependencies(${PROJECT_NAME} GeneratedPythonSources)

  if(RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE)
    add_dependencies(${PROJECT_NAME} PipInstall)
    add_dependencies(GeneratedPythonSources PipInstall)
  endif()
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  install(DIRECTORY ${GRPC_PYTHON_SOURCE_PATH}/
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/Python)
endif()

source_group(
  "GrpcInterface"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake)
source_group("GrpcInterface\\GrpcProtos" FILES ${GRPC_PROTO_FILES_FULL_PATH})
if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  source_group(
    TREE ${GRPC_PYTHON_SOURCE_PATH}
    FILES ${GRPC_PYTHON_SOURCES_FULL_PATH}
    PREFIX "GrpcInterface\\Python")
endif()

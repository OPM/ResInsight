project(GrpcInterface)

message(STATUS "GRPC enabled")
add_definitions(-DENABLE_GRPC)

if(RESINSIGHT_VCPKG_AUTO_INSTALL)
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

set(CMAKE_CXX_STANDARD 17)


add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

find_package(
  Qt5
  COMPONENTS Core Network
  QUIET)

set(QT_LIBRARIES
    Qt5::Core
    Qt5::Network)

set(SOURCE_GROUP_HEADER_FILES
    RiaGrpcServer.h
    RiaGrpcHelper.h
    RiaGrpcCallbacks.h
    RiaGrpcCallbacks.inl
    RiaGrpcServiceInterface.h
    RiaGrpcCaseService.h
    RiaGrpcSimulationWellService.h
    RiaGrpcGridService.h
    RiaGrpcProjectService.h
    RiaGrpcCommandService.h
    RiaGrpcAppService.h
    RiaGrpcPropertiesService.h
    RiaGrpcNNCPropertiesService.h
    RiaGrpcPdmObjectService.h
    RiaGrpcApplicationInterface.h
    RiaGrpcGridGeometryExtractionService.h
    RiaGrpcGridGeometryExtractionServiceHelper.h
    )

set(SOURCE_GROUP_SOURCE_FILES
    RiaGrpcServer.cpp
    RiaGrpcHelper.cpp
    RiaGrpcServiceInterface.cpp
    RiaGrpcCaseService.cpp
    RiaGrpcSimulationWellService.cpp
    RiaGrpcGridService.cpp
    RiaGrpcProjectService.cpp
    RiaGrpcCommandService.cpp
    RiaGrpcAppService.cpp
    RiaGrpcPropertiesService.cpp
    RiaGrpcNNCPropertiesService.cpp
    RiaGrpcPdmObjectService.cpp
    RiaGrpcApplicationInterface.cpp
    RiaGrpcGridGeometryExtractionService.cpp
    RiaGrpcGridGeometryExtractionServiceHelper.cpp
)

# Find Protobuf installation Looks for protobuf-config.cmake file installed by
# Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE ON)
find_package(Protobuf CONFIG 3.0 QUIET)

if(Protobuf_FOUND)
  message(STATUS "Using protobuf ${protobuf_VERSION}")

  # Find gRPC installation Looks for gRPCConfig.cmake file installed by gRPC's
  # cmake installation.
  find_package(gRPC CONFIG REQUIRED)
  message(STATUS "Generate C++ code using grpc : ${gRPC_VERSION}")

  set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
  set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

  set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure gRPC::grpc_unsecure gRPC::gpr)
  set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
  set(GRPC_LINK_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

  set_target_properties(
    ${GRPC_LINK_LIBRARIES} PROPERTIES MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
                                 MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)
else()
  message(STATUS "Using RESINSIGHT_GRPC_INSTALL_PREFIX=${RESINSIGHT_GRPC_INSTALL_PREFIX}")
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
  pkg_check_modules(GRPC REQUIRED grpc grpc++_unsecure>=1.20 grpc_unsecure gpr
                    protobuf libcares)
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

set(_LINK_LIBRARIES
    ${QT_LIBRARIES}
    ${GRPC_LINK_LIBRARIES}
    LibCore
    CommonCode
    cafCommand
    cafCommandFeatures
    cafProjectDataModel
    cafPdmScripting
    ApplicationLibCode)

# Proto files
set(GRPC_PROTO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/App.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Case.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Commands.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Definitions.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Grid.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/GridGeometryExtraction.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/NNCProperties.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/PdmObject.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Project.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/Properties.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/SimulationWell.proto
    ${CMAKE_CURRENT_SOURCE_DIR}/GrpcProtos/VectorDefines.proto
)

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Python")

add_custom_target(PipInstall)

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  message(STATUS "Using Python Executable: ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE}")
  
  execute_process(COMMAND "${RESINSIGHT_GRPC_PYTHON_EXECUTABLE}" -m grpc_tools.protoc --version
                  OUTPUT_VARIABLE PYTHON_GRPC_PROTOC_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  message(STATUS "Using Python grpc protoc compiler: ${PYTHON_GRPC_PROTOC_VERSION}")
else()
    message(STATUS "RESINSIGHT_GRPC_PYTHON_EXECUTABLE not specified. Will not install Python client code")
endif()

foreach(rips_proto ${GRPC_PROTO_FILES})
  get_filename_component(rips_proto_name ${rips_proto} NAME_WE)
  get_filename_component(rips_proto_path ${rips_proto} PATH)


  set(rips_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${rips_proto_name}.pb.cc")
  set(rips_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${rips_proto_name}.pb.h")
  set(rips_grpc_srcs "${CMAKE_BINARY_DIR}/Generated/${rips_proto_name}.grpc.pb.cc")
  set(rips_grpc_hdrs "${CMAKE_BINARY_DIR}/Generated/${rips_proto_name}.grpc.pb.h")

  add_custom_command(
    OUTPUT "${rips_proto_srcs}" "${rips_proto_hdrs}" "${rips_grpc_srcs}"
           "${rips_grpc_hdrs}"
    COMMAND
      ${_PROTOBUF_PROTOC} ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated"
      --cpp_out "${CMAKE_BINARY_DIR}/Generated" -I "${rips_proto_path}"
      --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}" "${rips_proto}"
    DEPENDS "${rips_proto}")

  set(rips_proto_python "rips/generated/${rips_proto_name}_pb2.py")
  set(rips_grpc_python "rips/generated/${rips_proto_name}_pb2_grpc.py")

  if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
    add_custom_command(
      OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${rips_proto_python}"
             "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}"
      COMMAND
        ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m grpc_tools.protoc -I
        "${rips_proto_path}" --python_out
        "${GRPC_PYTHON_SOURCE_PATH}/rips/generated" --grpc_python_out
        "${GRPC_PYTHON_SOURCE_PATH}/rips/generated" --pyi_out
        "${GRPC_PYTHON_SOURCE_PATH}/rips/generated" "${rips_proto}"
      DEPENDS "${rips_proto}"
      COMMENT "Generating ${rips_proto_python} and ${rips_grpc_python}"
      VERBATIM)
    list(APPEND GRPC_PYTHON_GENERATED_SOURCES "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}")
  endif()

  list(APPEND GRPC_HEADER_FILES ${rips_proto_hdrs} ${rips_grpc_hdrs})

  list(APPEND GRPC_CPP_SOURCES ${rips_proto_srcs} ${rips_grpc_srcs})

endforeach(rips_proto)

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  configure_file(${CMAKE_SOURCE_DIR}/ApplicationLibCode/Adm/RiaVersionInfo.py.cmake
                 ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/RiaVersionInfo.py)
  configure_file(${GRPC_PYTHON_SOURCE_PATH}/setup.py.cmake
                 ${GRPC_PYTHON_SOURCE_PATH}/setup.py)
endif()

list (APPEND GRPC_PYTHON_GENERATED_SOURCES ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/RiaVersionInfo.py)

file(GLOB GRPC_PYTHON_RIPS_SOURCES ${GRPC_PYTHON_SOURCE_PATH}/rips/*.py)
file(GLOB GRPC_PYTHON_EXAMPLE_SOURCES ${GRPC_PYTHON_SOURCE_PATH}/rips/PythonExamples/*.py)
file(GLOB GRPC_PYTHON_TEST_SOURCES ${GRPC_PYTHON_SOURCE_PATH}/rips/tests/*.py)
file(GLOB GRPC_PYTHON_OTHER_FILES  "${GRPC_PYTHON_SOURCE_PATH}/setup.py" "${GRPC_PYTHON_SOURCE_PATH}/requirements.txt" "${GRPC_PYTHON_SOURCE_PATH}/README.md" "${GRPC_PYTHON_SOURCE_PATH}/LICENSE")

list(
  APPEND
  GRPC_PYTHON_SOURCES
  ${GRPC_PYTHON_RIPS_SOURCES}
  ${GRPC_PYTHON_EXAMPLE_SOURCES}
  ${GRPC_PYTHON_TEST_SOURCES}
  ${GRPC_PYTHON_OTHER_FILES}
  ${GRPC_PROTO_FILES}
)

list(APPEND GRPC_PYTHON_SOURCES ${GRPC_PYTHON_GENERATED_SOURCES})

add_library(
  ${PROJECT_NAME} OBJECT
  ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
  ${GRPC_HEADER_FILES} ${GRPC_CPP_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
  ${GRPC_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/Generated
)

target_link_libraries(${PROJECT_NAME} PRIVATE ${_LINK_LIBRARIES})

if(MSVC)
  target_compile_options(
    GrpcInterface
    PRIVATE /wd4251 /wd4244 /wd4267)
else()
  target_compile_options(
    GrpcInterface
    PRIVATE -Wno-switch -Wno-overloaded-virtual
  )
endif()

# install gRPC Python files
if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE)
  message(STATUS "Installing Python modules (dev-requirements.txt)")
  add_custom_command(
    TARGET PipInstall
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install -r ${GRPC_PYTHON_SOURCE_PATH}/dev-requirements.txt --user )
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND RESINSIGHT_GRPC_BUNDLE_PYTHON_MODULE)
  message(STATUS "Bundling Python GRPC modules")
  add_custom_command(
    TARGET PipInstall
    COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} ARGS -m pip install --user
            --target=${GRPC_PYTHON_SOURCE_PATH} grpcio-tools)
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  add_custom_target(Rips ALL
                    SOURCES ${GRPC_PYTHON_SOURCES})
  add_dependencies(${PROJECT_NAME} Rips)

  if(RESINSIGHT_GRPC_DOWNLOAD_PYTHON_MODULE)
    add_dependencies(${PROJECT_NAME} PipInstall)
    add_dependencies(Rips PipInstall)
  endif()
endif()

if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
  install(DIRECTORY ${GRPC_PYTHON_SOURCE_PATH}/
          DESTINATION ${RESINSIGHT_INSTALL_FOLDER}/Python)
endif()

source_group("Generated" FILES ${GRPC_PYTHON_GENERATED_SOURCES})
source_group("Sources" FILES ${GRPC_PYTHON_RIPS_SOURCES})
source_group("Protos" FILES ${GRPC_PROTO_FILES})
source_group("PythonExamples" FILES ${GRPC_PYTHON_EXAMPLE_SOURCES})
source_group("Tests" FILES ${GRPC_PYTHON_TEST_SOURCES})

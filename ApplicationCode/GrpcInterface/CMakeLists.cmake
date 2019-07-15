cmake_minimum_required (VERSION 2.8.12)

set ( SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.inl
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCaseService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcAppService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPropertiesService.h
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPdmObjectService.h
)

set ( SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCaseService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcAppService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPropertiesService.cpp
	${CMAKE_CURRENT_LIST_DIR}/RiaGrpcPdmObjectService.cpp
)

add_definitions(-DENABLE_GRPC)

if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)		

    # Find Protobuf installation
    # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
    set(protobuf_MODULE_COMPATIBLE ON CACHE DBOOL "")
    find_package(Protobuf CONFIG 3.0 QUIET)
	if (Protobuf_FOUND)
		message(STATUS "Using protobuf ${protobuf_VERSION}")
	else()
		message(FATAL_ERROR "Protocol Buffers not found. This is required to build with gRPC")
    endif()

    # Find gRPC installation
    # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
    find_package(gRPC CONFIG REQUIRED NO_MODULE)
    message(STATUS "Using gRPC ${gRPC_VERSION}")

    set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

    set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure gRPC::grpc_unsecure gRPC::gpr)
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)	
    set(GRPC_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

    set_target_properties(${GRPC_LIBRARIES} PROPERTIES
        MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )

else()
    if (NOT DEFINED GRPC_INSTALL_PREFIX OR NOT EXISTS ${GRPC_INSTALL_PREFIX})
        message(FATAL_ERROR "You need a valid GRPC_INSTALL_PREFIX set to build with gRPC")
    endif()
    set(ENV{PKG_CONFIG_PATH} "${GRPC_INSTALL_PREFIX}/lib/pkgconfig")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRPC REQUIRED grpc++_unsecure>=1.20 grpc_unsecure gpr protobuf)
    set(_PROTOBUF_PROTOC "${GRPC_INSTALL_PREFIX}/bin/protoc")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE "${GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
    include_directories(AFTER ${GRPC_INCLUDE_DIRS})
endif()

# Cannot use the nice new FindPackage modules for python since that is CMake 3.12+
if(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
    message(STATUS "Using Python ${PYTHON_EXECUTABLE}")
endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

# Proto files
set(PROTO_FILES
    "Empty"
	"PdmObject"
    "Case"
    "Project"
    "Commands"
    "App"
	"Properties"
	"Grid"
)

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/Python")
set(GRPC_PYTHON_DEST_PATH "${CMAKE_BINARY_DIR}/Python")

foreach(proto_file ${PROTO_FILES})		
    get_filename_component(rips_proto "${CMAKE_CURRENT_LIST_DIR}/GrpcProtos/${proto_file}.proto" ABSOLUTE)
    get_filename_component(rips_proto_path "${rips_proto}" PATH)

	list(APPEND GRPC_PROTO_FILES_FULL_PATH ${rips_proto})

    set(rips_proto_srcs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.cc")
    set(rips_proto_hdrs "${CMAKE_BINARY_DIR}/Generated/${proto_file}.pb.h")
    set(rips_grpc_srcs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.cc")
    set(rips_grpc_hdrs  "${CMAKE_BINARY_DIR}/Generated/${proto_file}.grpc.pb.h")
    
    add_custom_command(
        OUTPUT "${rips_proto_srcs}" "${rips_proto_hdrs}" "${rips_grpc_srcs}" "${rips_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_BINARY_DIR}/Generated"
            --cpp_out "${CMAKE_BINARY_DIR}/Generated"
            -I "${rips_proto_path}"
            --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
            "${rips_proto}"
        DEPENDS "${rips_proto}"
    )
    
    if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
        set(rips_proto_python "rips/generated/${proto_file}_pb2.py")
        set(rips_grpc_python "rips/generated/${proto_file}_pb2_grpc.py")

        add_custom_command(
            OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${rips_proto_python}" "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}"
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS -m grpc_tools.protoc
                -I "${rips_proto_path}"
                --python_out "${GRPC_PYTHON_SOURCE_PATH}/rips/generated"
                --grpc_python_out "${GRPC_PYTHON_SOURCE_PATH}/rips/generated"
                "${rips_proto}"
            DEPENDS "${rips_proto}"
            COMMENT "Generating ${rips_proto_python} and ${rips_grpc_python}"
            VERBATIM
        )
        list (APPEND GRPC_PYTHON_GENERATED_SOURCES
            ${rips_proto_python}
            ${rips_grpc_python}
        )

    endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

    list( APPEND GRPC_HEADER_FILES
          ${rips_proto_hdrs}
          ${rips_grpc_hdrs}
    )
    
    list( APPEND GRPC_CPP_SOURCES
          ${rips_proto_srcs}
          ${rips_grpc_srcs}
    )

endforeach(proto_file)

if (PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})
    list(APPEND GRPC_PYTHON_SOURCES
        ${GRPC_PYTHON_GENERATED_SOURCES}
		"rips/generated/RiaVersionInfo.py"
        "rips/__init__.py"
		"rips/App.py"
		"rips/Case.py"
		"rips/Commands.py"
		"rips/Grid.py"
		"rips/Project.py"
		"rips/Properties.py"
        "rips/Instance.py"	
		"rips/PdmObject.py"
		"rips/View.py"
		"rips/examples/InstanceExample.py"
        "rips/examples/CommandExample.py"
        "rips/examples/CaseInfoStreamingExample.py"
		"rips/examples/SoilPorvAsync.py"
		"rips/examples/SoilPorvSync.py"
        "rips/examples/SelectedCases.py"
        "rips/examples/AllCases.py"
		"rips/examples/SetGridProperties.py"
		"rips/examples/GridInformation.py"
		"rips/examples/InputPropTestSync.py"
		"rips/examples/InputPropTestAsync.py"
		"rips/examples/SoilAverage.py"
		"rips/examples/SoilAverageNoComm.py"
        "rips/tests/test_cases.py"
		"rips/tests/test_commands.py"
		"rips/tests/test_grids.py"
		"rips/tests/test_properties.py"
		"rips/tests/test_project.py"
		"rips/tests/conftest.py"
		"rips/tests/dataroot.py"
		"requirements.txt"
		"setup.py.cmake"
		"README.md"
		"LICENSE"
    )

    foreach(PYTHON_SCRIPT ${GRPC_PYTHON_SOURCES})
        list(APPEND GRPC_PYTHON_SOURCES_FULL_PATH "${GRPC_PYTHON_SOURCE_PATH}/${PYTHON_SCRIPT}")
    endforeach()
if (MSVC)
    source_group(TREE ${GRPC_PYTHON_SOURCE_PATH} FILES ${GRPC_PYTHON_SOURCES_FULL_PATH} PREFIX "GrpcInterface\\Python")
endif(MSVC)

endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

list ( APPEND GRPC_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list ( APPEND GRPC_CPP_SOURCES ${SOURCE_GROUP_SOURCE_FILES})

CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.py.cmake
                ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/RiaVersionInfo.py
)
CONFIGURE_FILE( ${GRPC_PYTHON_SOURCE_PATH}/setup.py.cmake
                ${GRPC_PYTHON_SOURCE_PATH}/setup.py
)

source_group( "GrpcInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake )
source_group( "GrpcInterface\\GrpcProtos" FILES ${GRPC_PROTO_FILES_FULL_PATH} )
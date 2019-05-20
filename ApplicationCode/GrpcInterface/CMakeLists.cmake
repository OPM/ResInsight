set ( SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCallbacks.inl
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridInfoService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectInfoService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.h
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcResInfoService.h
)

set ( SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcServiceInterface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcGridInfoService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcProjectInfoService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcCommandService.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcResInfoService.cpp
)

add_definitions(-DENABLE_GRPC)

if (MSVC)
    add_definitions(-D_WIN32_WINNT=0x600)		

    # Find Protobuf installation
    # Looks for protobuf-config.cmake file installed by Protobuf's cmake installation.
    set(protobuf_MODULE_COMPATIBLE ON CACHE BOOL "")
    find_package(Protobuf CONFIG 3.0 REQUIRED)
    message(STATUS "Using protobuf ${protobuf_VERSION}")

    # Find gRPC installation
    # Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
    find_package(gRPC CONFIG REQUIRED NO_MODULE)
    message(STATUS "Using gRPC ${gRPC_VERSION}")

    set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
    set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

    set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure gRPC::grpc_unsecure gRPC::gpr)
    set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)	
    set(GRPC_LIBRARIES ${_GRPC_GRPCPP_UNSECURE} ${_PROTOBUF_LIBPROTOBUF})

    if (MSVC)
        set_target_properties(${GRPC_LIBRARIES} PROPERTIES
            MAP_IMPORTED_CONFIG_MINSIZEREL RELEASE
            MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
        )
    endif(MSVC)
else()
    if (NOT DEFINED GRPC_INSTALL_PREFIX OR NOT EXISTS ${GRPC_INSTALL_PREFIX})
        message(FATAL_ERROR "You need a valid GRPC_INSTALL_PREFIX set to build with GRPC")
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
    "CaseInfo"
    "GridInfo"
    "ProjectInfo"
    "Commands"
    "ResInfo"
)

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_SOURCE_DIR}/Python")
set(GRPC_PYTHON_DEST_PATH "${CMAKE_BINARY_DIR}/Python")

foreach(proto_file ${PROTO_FILES})		
    get_filename_component(rips_proto "${CMAKE_CURRENT_LIST_DIR}/GrpcProtos/${proto_file}.proto" ABSOLUTE)
    get_filename_component(rips_proto_path "${rips_proto}" PATH)

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
        set(rips_proto_python "generated/${proto_file}_pb2.py")
        set(rips_grpc_python "generated/${proto_file}_pb2_grpc.py")

        add_custom_command(
            OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${rips_proto_python}" "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}"
            COMMAND ${PYTHON_EXECUTABLE}
            ARGS -m grpc_tools.protoc
                -I "${rips_proto_path}"
                --python_out "${GRPC_PYTHON_SOURCE_PATH}/generated"
                --grpc_python_out "${GRPC_PYTHON_SOURCE_PATH}/generated"
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
        "api/__init__.py"
        "api/ResInsight.py"
        "examples/CommandExample.py"
        "examples/GridInfoStreamingExample.py"
        "examples/ResultValues.py"
        "examples/SelectedCases.py"
        "examples/AllCases.py"
        "tests/test_sample.py"
    )

    foreach(PYTHON_SCRIPT ${GRPC_PYTHON_SOURCES})
        list(APPEND GRPC_PYTHON_SOURCES_FULL_PATH "${GRPC_PYTHON_SOURCE_PATH}/${PYTHON_SCRIPT}")
    endforeach()
    source_group(TREE ${GRPC_PYTHON_SOURCE_PATH} FILES ${GRPC_PYTHON_SOURCES_FULL_PATH} PREFIX "Python")

endif(PYTHON_EXECUTABLE AND EXISTS ${PYTHON_EXECUTABLE})

list ( APPEND GRPC_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list ( APPEND GRPC_CPP_SOURCES ${SOURCE_GROUP_SOURCE_FILES})

source_group( "GrpcInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake )

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
    set(RESINSIGHT_GRPC_INSTALL_PREFIX "" CACHE PATH "gRPC : Install prefix for gRPC")
    if (NOT DEFINED RESINSIGHT_GRPC_INSTALL_PREFIX OR NOT EXISTS ${RESINSIGHT_GRPC_INSTALL_PREFIX})
        message(FATAL_ERROR "You need a valid RESINSIGHT_GRPC_INSTALL_PREFIX set to build with gRPC")
    endif()
    set(ENV{PKG_CONFIG_PATH} "${RESINSIGHT_GRPC_INSTALL_PREFIX}/lib/pkgconfig")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRPC REQUIRED grpc++_unsecure>=1.20 grpc_unsecure gpr protobuf)
    set(_PROTOBUF_PROTOC "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/protoc")
    set(_GRPC_CPP_PLUGIN_EXECUTABLE "${RESINSIGHT_GRPC_INSTALL_PREFIX}/bin/grpc_cpp_plugin")
    include_directories(AFTER ${GRPC_INCLUDE_DIRS})
endif()

# Cannot use the nice new FindPackage modules for python since that is CMake 3.12+
if(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
    message(STATUS "Using Python ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE}")
endif(RESINSIGHT_GRPC_PYTHON_EXECUTABLE AND EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})

# Proto files
set(PROTO_FILES
    "Definitions"
	"PdmObject"
    "Case"
    "Project"
    "Commands"
    "App"
	"Properties"
	"Grid"
)

set(GRPC_PYTHON_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/Python")

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
    
    if (RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
		if (EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
			set(rips_proto_python "rips/generated/${proto_file}_pb2.py")
			set(rips_grpc_python "rips/generated/${proto_file}_pb2_grpc.py")

			add_custom_command(
				OUTPUT "${GRPC_PYTHON_SOURCE_PATH}/${rips_proto_python}" "${GRPC_PYTHON_SOURCE_PATH}/${rips_grpc_python}"
				COMMAND ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE}
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
		else()
			message(STATUS "Error generating Python for ${rips_proto}: RESINSIGHT_GRPC_PYTHON_EXECUTABLE set but ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} not found")
		endif(EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
	else()
		message(STATUS "RESINSIGHT_GRPC_PYTHON_EXECUTABLE not specified. Will not generate GRPC Python code.")
	endif(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)	

    list( APPEND GRPC_HEADER_FILES
          ${rips_proto_hdrs}
          ${rips_grpc_hdrs}
    )
    
    list( APPEND GRPC_CPP_SOURCES
          ${rips_proto_srcs}
          ${rips_grpc_srcs}
    )

endforeach(proto_file)

if (RESINSIGHT_GRPC_PYTHON_EXECUTABLE)
	if (EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
		CONFIGURE_FILE( ${CMAKE_SOURCE_DIR}/ApplicationCode/Adm/RiaVersionInfo.py.cmake
		                ${GRPC_PYTHON_SOURCE_PATH}/rips/generated/RiaVersionInfo.py)
		CONFIGURE_FILE( ${GRPC_PYTHON_SOURCE_PATH}/setup.py.cmake
                        ${GRPC_PYTHON_SOURCE_PATH}/setup.py)

	    list(APPEND GRPC_PYTHON_SOURCES
			"rips/generated/RiaVersionInfo.py"
			"rips/__init__.py"
			"rips/case.py"
			"rips/grid.py"
			"rips/gridcasegroup.py"
			"rips/project.py"
			"rips/instance.py"	
			"rips/pdmobject.py"
			"rips/view.py"
			"rips/PythonExamples/InstanceExample.py"
			"rips/PythonExamples/CommandExample.py"
			"rips/PythonExamples/CaseGridGroup.py"
			"rips/PythonExamples/CaseInfoStreamingExample.py"
			"rips/PythonExamples/ExportSnapshots.py"
			"rips/PythonExamples/ErrorHandling.py"
			"rips/PythonExamples/SoilPorvAsync.py"
			"rips/PythonExamples/SoilPorvSync.py"
			"rips/PythonExamples/SelectedCases.py"
			"rips/PythonExamples/AllCases.py"
			"rips/PythonExamples/SetGridProperties.py"
			"rips/PythonExamples/SetCellResult.py"
			"rips/PythonExamples/SetFlowDiagnosticsResult.py"
			"rips/PythonExamples/GridInformation.py"
			"rips/PythonExamples/InputPropTestSync.py"
			"rips/PythonExamples/InputPropTestAsync.py"
			"rips/PythonExamples/SoilAverageAsync.py"
			"rips/PythonExamples/SoilAverageSync.py"
			"rips/PythonExamples/ViewExample.py"
			"rips/tests/test_cases.py"
			"rips/tests/test_grids.py"
			"rips/tests/test_properties.py"
			"rips/tests/test_project.py"
			"rips/tests/conftest.py"
			"rips/tests/dataroot.py"
			"requirements.txt"
			"setup.py"
			"README.md"
			"LICENSE"
		)
		
		list(APPEND GRPC_PYTHON_SOURCES ${GRPC_PYTHON_GENERATED_SOURCES})

		foreach(PYTHON_SCRIPT ${GRPC_PYTHON_SOURCES})
			list(APPEND GRPC_PYTHON_SOURCES_FULL_PATH "${GRPC_PYTHON_SOURCE_PATH}/${PYTHON_SCRIPT}")		
		endforeach()

		if (MSVC)
			source_group(TREE ${GRPC_PYTHON_SOURCE_PATH} FILES ${GRPC_PYTHON_SOURCES_FULL_PATH} PREFIX "GrpcInterface\\Python")
		endif(MSVC)
	else()
		message(STATUS "Error copying GRPC Python Code to build folder: RESINSIGHT_GRPC_PYTHON_EXECUTABLE set but ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE} not found")
	endif(EXISTS ${RESINSIGHT_GRPC_PYTHON_EXECUTABLE})
else()
	message(STATUS "RESINSIGHT_GRPC_PYTHON_EXECUTABLE not specified. Will not copy grpc Python code to build folder")
endif(RESINSIGHT_GRPC_PYTHON_EXECUTABLE)	

list ( APPEND GRPC_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list ( APPEND GRPC_CPP_SOURCES ${SOURCE_GROUP_SOURCE_FILES})

source_group( "GrpcInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.cmake )
source_group( "GrpcInterface\\GrpcProtos" FILES ${GRPC_PROTO_FILES_FULL_PATH} )